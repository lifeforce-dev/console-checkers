import configparser
import dearpygui.dearpygui as dpg
from enum import Enum
from git_helper import GitHelper
import json
import os
from pathlib import Path
from tkinter import filedialog, Tk
import re
import shutil
import subprocess


SLN_DIR = Path(os.path.dirname(os.path.abspath(__file__))).parent
PACKAGE_STORE_PATH = SLN_DIR / 'premake' / 'package_store.json'
DEPENDENCIES_PATH = SLN_DIR / 'dependencies.json'


# UI defaults
NO_OUTPUT_DIR_SELECTED_TEXT = "Choose sln output dir..."
DISABLED_COLOR = (0.50 * 255, 0.50 * 255, 0.50 * 255, 1.00 * 255)
DISABLED_BUTTON_COLOR = (45, 45, 48)
DISABLED_BUTTON_HOVER_COLOR = (45, 45, 48)
DISABLED_BUTTON_ACTIVE_COLOR = (45, 45, 48)

STATUS_TEXT_PREFIX = "Working..."
STATUS_TEXT_ERROR_PREFIX = "Error:"

class Mode(Enum):
    CREATE_NEW = 1,
    UPDATE = 2


class SolutionNameMissingException(Exception):
    def __init__(self, message="Solution name cannot be empty"):
        self.message = message
        super().__init__(self.message)


class PackageCacheNotSetError(Exception):
    def __init__(self):
        message = ("The 'PACKAGE_CACHE_PATH' environment variable is not set."
                   " Please set this variable to the path of your package cache.")
        super().__init__(message)


class DirectoryAlreadyExistsError(Exception):
    def __init__(self, directory, message="Directory already exists. directory="):
        self.directory = directory
        self.message = f"{message}: {directory}"
        super().__init__(self.message)


class PackageCheckBoxItem:
    def __init__(self, package_name, checkbox_id, is_checked=False):
        self.package_name = package_name
        self.checkbox_id = checkbox_id
        self.is_checked = is_checked


class PackageDropDownItem:
    def __init__(self, versions, dropdown_id, current_version):
        self.versions = versions
        self.dropdown_id = dropdown_id
        self.current_version = current_version


class PackageCheckBoxGroupItem:
    def __init__(self, checkbox_item, dropdown_item, package_name):
        self.package_name = package_name
        self.checkbox_item = checkbox_item
        self.dropdown_item = dropdown_item


class PackageSelectorGUI:
    def __init__(self):
        self.selected_packages = set()
        self.solution_name = ""
        self.checkbox_ids = {}
        self.dropdown_ids = {}
        self.package_items = {}
        self.window_id = None
        self.output_dir_label_id = None
        self.generate_button_id = None
        self.solution_name_input_id = None
        self.update_button_id = None
        self.browse_button_id = None
        self.status_text_id = None
        self.dependencies = {}
        self.package_store = {}
        self.mode = Mode.CREATE_NEW
        self.output_dir = ""
        self.create_gui()


    def load_package_store(self):
        try:
            with open(PACKAGE_STORE_PATH, encoding='utf-8') as file:
                self.package_store = json.load(file)['package_store']
        except FileNotFoundError:
            self.package_store = {}


    def load_dependencies(self):
        try:
            with open(DEPENDENCIES_PATH, encoding='utf-8') as file:
                self.dependencies = json.load(file)
        except FileNotFoundError:
            self.dependencies = {}

    def create_default_settings(self):
        config = configparser.ConfigParser()
        config['DEFAULT'] = {
            'mode': 'CREATE_NEW'
        }
        with open(SLN_DIR / 'settings.ini', 'w') as configfile:
            config.write(configfile)


    def initialize(self):
        if not os.path.exists(SLN_DIR / 'settings.ini'):
            self.create_default_settings()

        config_parser = configparser.ConfigParser()
        config_parser.read(SLN_DIR/'settings.ini')

        mode_str = config_parser.get('DEFAULT', 'mode', fallback="GENERATE")
        self.mode = Mode[mode_str]

        # If mode is UPDATE we don't care about output_dir because we only operate in current dir.
        if self.mode == Mode.CREATE_NEW:
            self.output_dir = config_parser.get('DEFAULT', 'output_dir',
                                                fallback= NO_OUTPUT_DIR_SELECTED_TEXT)
        elif self.mode == Mode.UPDATE:
            self.solution_name = config_parser.get('DEFAULT', 'solution_name', fallback="default")
        self.load_package_store()
        self.load_dependencies()


    def create_gui(self):
        self.initialize()
        self.window_id = dpg.add_window(label="Package Selector", no_scrollbar=True,
                                        menubar=False, no_resize=True, no_move=True)
        with dpg.window(id=self.window_id):

            # Output dir select button
            if self.mode == Mode.CREATE_NEW:
                with dpg.group(horizontal=True):
                    self.browse_button_id = dpg.add_button(label="Browse", callback=self.on_choose_output_dir)

                    # Current output dir label
                    self.output_dir_label_id = dpg.add_text(
                        self.output_dir if self.output_dir.strip(' "') != "" else NO_OUTPUT_DIR_SELECTED_TEXT)

                # Solution name Edit box
                self.solution_name_input_id = dpg.add_input_text(hint="Enter Solution Name")
                dpg.set_item_callback(self.solution_name_input_id, self.on_solution_text_changed)

            dpg.add_text("Packages")
            # Package Items
            for package_name, package_info in self.package_store.items():
                with dpg.group(horizontal=True):
                    # Checkbox
                    checkbox_id = dpg.add_checkbox(label=package_name,
                                                   callback=self.on_checkbox_checked,
                                                   user_data=package_name)
                    self.checkbox_ids[checkbox_id] = package_name

                    is_checked = package_name in self.dependencies

                    checkbox_item = PackageCheckBoxItem(package_name, checkbox_id, is_checked=is_checked)

                    dpg.set_value(checkbox_id, is_checked)

                    # Dropdown
                    dropdown_id = dpg.add_combo(package_info['versions'],
                                                default_value=self.dependencies.get(package_name, package_info['versions'][0]),
                                                user_data=package_name,
                                                callback=self.on_dropdown_changed)
                    self.dropdown_ids[dropdown_id] = package_name
                    current_version = self.dependencies.get(package_name, package_info['versions'][0])
                    dropdown_item = PackageDropDownItem(package_info['versions'], dropdown_id, current_version)

                    group_item = PackageCheckBoxGroupItem(checkbox_item, dropdown_item, package_name)
                    self.package_items[package_name] = group_item

            # Generate Button or Update Button
            self.create_execute_button()
            self.status_text_id = dpg.add_text("", wrap=500)


    def create_execute_button(self):
        if self.mode == Mode.CREATE_NEW:
            self.generate_button_id = dpg.add_button(label="Generate", callback=self.on_generate_clicked)
            dpg.disable_item(self.generate_button_id)

            dpg.set_item_user_data(self.solution_name_input_id, self.generate_button_id)

        elif self.mode == Mode.UPDATE:
            self.update_button_id = dpg.add_button(label="Update", callback=self.on_update_clicked)


    def on_choose_output_dir(self, sender, app_data, user_data):
        root = Tk()
        directory = filedialog.askdirectory()
        root.destroy()

        # Update directory if we picked one
        if directory:
            self.output_dir = directory
            config_parser = configparser.ConfigParser()
            config_parser.read(SLN_DIR / 'settings.ini')
            config_parser.set('DEFAULT', 'output_dir', directory)
            with open(SLN_DIR / 'settings.ini', 'w', encoding='utf-8') as config_file:
                config_parser.write(config_file)
            dpg.set_value(self.output_dir_label_id, directory)
            self.update_generate_button_is_enabled()


    def on_dropdown_changed(self, sender, app_data, user_data):
        self.update_dependencies()


    def on_checkbox_checked(self, sender, app_data, user_data):
        package = user_data
        check_state_str = "Checked" if app_data else "Unchecked"
        print(f"{check_state_str} {package}")
        if app_data:
            self.selected_packages.add(package)
        else:
            self.selected_packages.discard(package)

        self.update_dependencies()

    def on_generate_clicked(self, sender, app_data, user_data):
        dpg.set_value(self.status_text_id, "")
        self.set_ui_enabled(False)
        self.solution_name = dpg.get_value(self.solution_name_input_id)
        try:
            if not self.solution_name.strip():
                raise SolutionNameMissingException
        except SolutionNameMissingException as e:
            print(e)
            return

        self.fetch_packages(self.get_checked_packages())
        self.generate_package_info_lua()
        dpg.set_value(self.status_text_id,f"{STATUS_TEXT_PREFIX} Creating folder structure...")

        self.solution_dir = Path(self.output_dir) / self.solution_name
        self.build_sln_dir(self.solution_dir)
        self.execute_premake(self.solution_dir)
        self.generate_package_manager_batch_script(self.solution_dir)


    def on_update_clicked(self, sender, app_data, user_data):
        self.fetch_packages(self.get_checked_packages())
        self.execute_premake(SLN_DIR)


    def on_solution_text_changed(self, sender, app_data, user_data):
        self.solution_name = dpg.get_value(sender)
        self.update_generate_button_is_enabled()
        dpg.set_value(self.status_text_id, "")


    def update_generate_button_is_enabled(self):
        if self.solution_name.strip() and os.path.isdir(self.output_dir):
            dpg.enable_item(self.generate_button_id)
        else:
            dpg.disable_item(self.generate_button_id)


    def set_ui_enabled(self, enabled):

        def safe_configure_item(item_id, enabled):
            if item_id is not None:
                dpg.configure_item(item_id, enabled=enabled)

        # Enable or disable UI elements safely
        safe_configure_item(self.browse_button_id, enabled)
        safe_configure_item(self.solution_name_input_id, enabled)
        safe_configure_item(self.generate_button_id, enabled)
        safe_configure_item(self.update_button_id, enabled)
        for package_item in self.package_items.values():
            safe_configure_item(package_item.checkbox_item.checkbox_id, enabled)
            safe_configure_item(package_item.dropdown_item.dropdown_id, enabled)


    def fetch_packages(self, checked_packages):
        package_cache_path_str = os.getenv('PACKAGE_CACHE_PATH')
        if not package_cache_path_str:
            dpg.set_value(self.status_text_id,
                          f"{STATUS_TEXT_ERROR_PREFIX} PACKAGE_CACHE_PATH env variable not set.")
            raise PackageCacheNotSetError()

        package_cache_path = Path(package_cache_path_str)

        for package_name, version in checked_packages:
            version = version.split('|')[1] if '|' in version else version
            repo_path = package_cache_path / package_name / version / package_name
            repo_url = self.package_store[package_name]['git_url']
            print(f"attempting repo={package_name} version={version}")

            if GitHelper.does_repo_exist(repo_path):
                dpg.set_value(self.status_text_id,f"{STATUS_TEXT_PREFIX} Updating {package_name}...")
                print(f"Repo exists at {repo_path}. Ensuring up to date.")
                if GitHelper.is_correct_version(repo_path, version):
                    GitHelper.reset_hard(repo_path)
                    GitHelper.pull(repo_path)
                else:
                    GitHelper.checkout(repo_path, version)
            else:
                dpg.set_value(self.status_text_id,f"{STATUS_TEXT_PREFIX} Cloning {package_name}...")
                GitHelper.clone(repo_path, repo_url)
                dpg.set_value(self.status_text_id,f"{STATUS_TEXT_PREFIX} Checking out branch {version}...")
                GitHelper.checkout(repo_path, version)


    def build_sln_dir(self, solution_dir):
        # I'm gonna error here because I want the user to decide what to do.
        # I don't want to just overwrite or delete their stuff.
        try:
            if solution_dir.exists() and os.listdir(solution_dir):
                dpg.set_value(self.status_text_id,
                            f"{STATUS_TEXT_ERROR_PREFIX} Failed to create solution at {solution_dir}."
                             " The destination must be empty.")
                raise DirectoryAlreadyExistsError(solution_dir)
        except DirectoryAlreadyExistsError:
            self.set_ui_enabled(True)
            return

        solution_dir.mkdir(parents=True, exist_ok=True)
        print("Creating directory at")

        # These files are needed go properly generate and update project files.
        shutil.copytree(SLN_DIR / 'premake', solution_dir / 'premake', dirs_exist_ok=True)
        shutil.copy2(SLN_DIR / 'dependencies.json', solution_dir)
        shutil.copy2(SLN_DIR / 'premake5.lua', solution_dir)
        shutil.copytree(SLN_DIR / 'scripts', solution_dir / 'scripts', dirs_exist_ok=True)

        # Might as well copy this over too, as it includes a lot of script/premake related paths.
        shutil.copy2(SLN_DIR / '.gitignore', solution_dir)

        # Create settings.ini and Source directory
        with open(solution_dir / 'settings.ini', 'w', encoding='utf-8') as file:
            file.write("[DEFAULT]\n")
            file.write("mode = UPDATE\n")
            file.write(f"solution_name = {self.solution_name}\n")
        (solution_dir / 'Source').mkdir()


    def generate_package_info_lua(self):
        # Define the path for package_info.lua
        package_info_lua_path = SLN_DIR / 'premake' / 'generated' / 'package_info.lua'
        package_info_lua_path.parent.mkdir(parents=True, exist_ok=True)

        # Building the dictionary for package info
        packages_dict = {}
        checked_packages = self.get_checked_packages()
        for package_name, version in checked_packages:
            clean_version = version.replace("git|", "")
            # Retrieve the package data from package_store using the package name
            package_data = self.package_store.get(package_name)

            if package_data:
                # Add to packages_dict
                packages_dict[package_name] = {
                    "version": clean_version,
                }

        # Convert the dictionary to a JSON string and format it for Lua
        package_info_json = json.dumps({"packages": packages_dict}, indent=4)
        package_info_lua = re.sub(r'\"([^\"]+)\":', r'\1 =', package_info_json)
        package_info_content = "return {" + package_info_lua[1:-1] + "}\n"

        # Write the content to package_info.lua
        with open(package_info_lua_path, 'w', encoding='utf-8') as file:
            file.write(package_info_content)


    def execute_premake(self, solution_dir):
        os.chdir(solution_dir)
        print(f"Project generated at {solution_dir}")
        print(f"Locating Premake")
        premake_executable = solution_dir / 'premake' / 'premake5.exe'
        if premake_executable.exists():
            try:
                command = [
                    str(premake_executable),
                    f'--sln_dir={str(solution_dir)}',
                    f'--sln_name={self.solution_name}',
                    "vs2022"
                ]

                print("Running Premake with command:", " ".join(command))
                result = subprocess.run(command, check=True, capture_output=True, text=True)

                print(f"Premake output: {result.stdout}")
            except subprocess.CalledProcessError as e:
                print(f"Error running premake: {e.output}")
                dpg.set_value(self.status_text_id, f"{STATUS_TEXT_ERROR_PREFIX} Error running premake.")
        else:
            print(f"Premake executable not found at {premake_executable}.")
            dpg.set_value(self.status_text_id, f"{STATUS_TEXT_ERROR_PREFIX} Premake executable not found.")
        dpg.set_value(self.status_text_id, "Done.")
        self.set_ui_enabled(True)
        os.chdir(SLN_DIR)


    def generate_package_manager_batch_script(self, solution_dir):
        os.chdir(solution_dir)
        try:
            batch_file_path = solution_dir / 'run_bootstrapper.bat'
            python_script_path = solution_dir / 'scripts' / 'bootstrapper.py'
            with open(batch_file_path, 'w') as batch_file:
                batch_file.write(f'@echo off\n')
                batch_file.write(f'python "{python_script_path}"\n')

                # TODO: Keeping the command window open for logs. Maybe make this configurable.
                batch_file.write(f'pause\n')

            print(f"Batch file written at {batch_file_path}")

        except Exception as e:
            print(f"Error: {e}")

        finally:
            os.chdir(SLN_DIR)


    def get_checked_packages(self):
        checked_packages = [
            (item.package_name, dpg.get_value(item.dropdown_item.dropdown_id))
            for item in self.package_items.values()
            if dpg.get_value(item.checkbox_item.checkbox_id)
        ]
        return checked_packages


    def update_dependencies(self):
        # This is the simplest and least error-prone way to update our depdencies.
        # Is it the most efficient? No, but we're talking about dozens of dependencies at most.
        updated_dependencies = {}

        for package_name, package_item in self.package_items.items():
            if dpg.get_value(package_item.checkbox_item.checkbox_id):
                dropdown_value = dpg.get_value(package_item.dropdown_item.dropdown_id)
                updated_dependencies[package_name] = dropdown_value

        # Update the dependencies file
        with open(SLN_DIR / 'dependencies.json', 'w', encoding='utf-8') as file:
            json.dump(updated_dependencies, file, indent=4)

        dpg.set_value(self.status_text_id, "")


def set_disabled_theme(component, theme):
    with dpg.theme_component(component, enabled_state=False, parent=theme):
        dpg.add_theme_color(dpg.mvThemeCol_Text, DISABLED_COLOR, category=dpg.mvThemeCat_Core)
        dpg.add_theme_color(dpg.mvThemeCol_Button, DISABLED_BUTTON_COLOR, category=dpg.mvThemeCat_Core)
        dpg.add_theme_color(dpg.mvThemeCol_ButtonHovered, DISABLED_BUTTON_HOVER_COLOR, category=dpg.mvThemeCat_Core)
        dpg.add_theme_color(dpg.mvThemeCol_ButtonActive, DISABLED_BUTTON_ACTIVE_COLOR, category=dpg.mvThemeCat_Core)


def main():
    dpg.create_context()
    gui = PackageSelectorGUI()

    dpg.create_viewport(title='Package Selector', width=600, height=300)
    dpg.setup_dearpygui()

    if gui.window_id is not None:
        dpg.set_primary_window(gui.window_id, True)

    with dpg.theme() as disabled_theme:
        component_list = [
            dpg.mvButton,
            dpg.mvCheckbox,
            dpg.mvCombo,
            dpg.mvText,
            dpg.mvInputText,
        ]
        for component in component_list:
            set_disabled_theme(component, disabled_theme)

    dpg.bind_theme(disabled_theme)

    dpg.show_viewport()
    dpg.start_dearpygui()
    dpg.destroy_context()

if __name__ == "__main__":
    main()
