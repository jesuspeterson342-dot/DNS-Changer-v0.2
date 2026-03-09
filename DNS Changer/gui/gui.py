#!/usr/bin/env python3

import ipaddress
import os
import shutil
import subprocess
import sys
import threading
from datetime import datetime
from pathlib import Path

try:
    import customtkinter as ctk
except ImportError:
    print("Error: customtkinter module not found!")
    print("\nPlease install it using one of these methods:")
    print("  1. For current user:")
    print("     pip3 install customtkinter")
    print("  2. System-wide (if running with sudo):")
    print("     sudo pip3 install customtkinter")
    print("  3. Using virtual environment (recommended):")
    print("     python3 -m venv venv")
    print("     source venv/bin/activate")
    print("     pip install customtkinter")
    print("     Then run: sudo -E env PATH=$PATH python3 gui/gui.py")
    sys.exit(1)


ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("dark-blue")


class DNSChangerApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("DNS Changer")
        self.geometry("400x500")
        self.minsize(400, 500)
        self.resizable(False, False)

        self.title_font = ctk.CTkFont(size=24, weight="bold")
        self.section_font = ctk.CTkFont(size=16, weight="bold")
        self.button_font = ctk.CTkFont(size=15, weight="bold")
        self.text_font = ctk.CTkFont(size=12)

        self.action_widgets = []
        self.dnschanger_path = self._find_dnschanger()

        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(3, weight=1)

        title_label = ctk.CTkLabel(self, text="DNS Changer", font=self.title_font)
        title_label.grid(row=0, column=0, padx=20, pady=(20, 10), sticky="ew")

        self._build_quick_dns_section()
        self._build_custom_dns_section()
        self._build_output_section()

        self._check_initial_status()

    def _find_dnschanger(self):
        """Find the dnschanger executable in various locations."""
        # Check if running as root
        if os.geteuid() != 0:
            return None

        # Priority order: installed -> local build -> project build
        possible_paths = [
            "/usr/local/bin/dnschanger",  # Installed location
            shutil.which("dnschanger"),    # In PATH
            "./dnschanger",                # Current directory
            "./bulid/my_program.bin",      # Project build directory (note: typo in original)
            "./build/my_program.bin",      # Corrected build directory
            str(Path(__file__).parent.parent / "dnschanger"),  # Relative to gui dir
            str(Path(__file__).parent.parent / "bulid" / "my_program.bin"),
            str(Path(__file__).parent.parent / "build" / "my_program.bin"),
        ]

        for path in possible_paths:
            if path and os.path.isfile(path) and os.access(path, os.X_OK):
                return path

        return None

    def _check_initial_status(self):
        """Check initial status and display appropriate messages."""
        if self.dnschanger_path is None:
            self.append_output(
                "ERROR: dnschanger executable not found!\n\n"
                "Please compile and install the program first:\n"
                "  sudo ./install.sh\n\n"
                "Or compile manually:\n"
                "  g++ -std=c++17 -O2 src/main.cpp -o dnschanger\n"
                "  sudo cp dnschanger /usr/local/bin/\n\n"
                "All functions are disabled."
            )
            self.set_busy(True)
            return

        self.append_output(
            f"Ready.\n"
            f"Using: {self.dnschanger_path}"
        )

    def _build_quick_dns_section(self):
        frame = ctk.CTkFrame(self, corner_radius=18)
        frame.grid(row=1, column=0, padx=20, pady=10, sticky="ew")
        frame.grid_columnconfigure(0, weight=1)

        label = ctk.CTkLabel(frame, text="Quick DNS", font=self.section_font)
        label.grid(row=0, column=0, padx=16, pady=(14, 10), sticky="w")

        cloudflare_btn = self._make_button(
            frame, "Cloudflare (1.1.1.1)", lambda: self.set_dns("1.1.1.1")
        )
        cloudflare_btn.grid(row=1, column=0, padx=16, pady=6, sticky="ew")

        google_btn = self._make_button(
            frame, "Google (8.8.8.8)", lambda: self.set_dns("8.8.8.8")
        )
        google_btn.grid(row=2, column=0, padx=16, pady=6, sticky="ew")

        quad9_btn = self._make_button(
            frame, "Quad9 (9.9.9.9)", lambda: self.set_dns("9.9.9.9")
        )
        quad9_btn.grid(row=3, column=0, padx=16, pady=(6, 14), sticky="ew")

    def _build_custom_dns_section(self):
        frame = ctk.CTkFrame(self, corner_radius=18)
        frame.grid(row=2, column=0, padx=20, pady=10, sticky="ew")
        frame.grid_columnconfigure(0, weight=1)

        label = ctk.CTkLabel(frame, text="Custom DNS", font=self.section_font)
        label.grid(row=0, column=0, padx=16, pady=(14, 10), sticky="w")

        self.dns_entry = ctk.CTkEntry(
            frame,
            placeholder_text="Enter DNS IP address",
            height=40,
            corner_radius=12,
            font=self.text_font,
        )
        self.dns_entry.grid(row=1, column=0, padx=16, pady=6, sticky="ew")
        self.action_widgets.append(self.dns_entry)

        apply_btn = self._make_button(frame, "Apply DNS", self.apply_custom_dns)
        apply_btn.grid(row=2, column=0, padx=16, pady=6, sticky="ew")

        restore_btn = self._make_button(frame, "Restore DNS", self.restore_dns)
        restore_btn.grid(row=3, column=0, padx=16, pady=6, sticky="ew")

        status_btn = self._make_button(frame, "Check current DNS", self.check_status)
        status_btn.grid(row=4, column=0, padx=16, pady=(6, 14), sticky="ew")

    def _build_output_section(self):
        frame = ctk.CTkFrame(self, corner_radius=18)
        frame.grid(row=3, column=0, padx=20, pady=(10, 20), sticky="nsew")
        frame.grid_columnconfigure(0, weight=1)
        frame.grid_rowconfigure(1, weight=1)

        label = ctk.CTkLabel(frame, text="Output", font=self.section_font)
        label.grid(row=0, column=0, padx=16, pady=(14, 10), sticky="w")

        self.output_box = ctk.CTkTextbox(
            frame,
            corner_radius=12,
            font=self.text_font,
            wrap="word",
        )
        self.output_box.grid(row=1, column=0, padx=16, pady=(0, 14), sticky="nsew")
        self.output_box.configure(state="disabled")

    def _make_button(self, parent, text, command):
        button = ctk.CTkButton(
            parent,
            text=text,
            command=command,
            height=42,
            corner_radius=14,
            font=self.button_font,
        )
        self.action_widgets.append(button)
        return button

    def append_output(self, message):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.output_box.configure(state="normal")
        self.output_box.insert("end", f"[{timestamp}] {message.strip()}\n\n")
        self.output_box.see("end")
        self.output_box.configure(state="disabled")

    def set_busy(self, busy):
        state = "disabled" if busy else "normal"
        for widget in self.action_widgets:
            widget.configure(state=state)

    def run_command(self, args, title_text):
        # Check if we have a valid dnschanger path
        if self.dnschanger_path is None:
            self.append_output(
                "Error:\nCannot execute command - dnschanger not found.\n"
                "Please install the program first."
            )
            return

        self.set_busy(True)
        self.append_output(f"{title_text}\n$ {' '.join(args)}")

        def worker():
            try:
                result = subprocess.run(
                    args,
                    capture_output=True,
                    text=True,
                    timeout=30,
                )

                parts = []
                if result.stdout.strip():
                    parts.append(result.stdout.strip())
                if result.stderr.strip():
                    parts.append(result.stderr.strip())
                if not parts:
                    parts.append("No output.")

                if result.returncode == 0:
                    message = "\n\n".join(parts)
                else:
                    message = (
                            f"Command failed with exit code {result.returncode}.\n\n"
                            + "\n\n".join(parts)
                    )

            except FileNotFoundError:
                message = (
                    f'Error:\nThe executable "{args[0]}" was not found.\n'
                    "Make sure it is installed and available."
                )
            except subprocess.TimeoutExpired:
                message = "Error:\nThe command took too long and was stopped."
            except Exception as exc:
                message = f"Error:\n{exc}"

            self.after(0, lambda: self._finish_command(message))

        threading.Thread(target=worker, daemon=True).start()

    def _finish_command(self, message):
        self.append_output(message)
        self.set_busy(False)

    def set_dns(self, dns):
        self.run_command([self.dnschanger_path, "set", dns], f"Applying DNS: {dns}")

    def apply_custom_dns(self):
        dns = self.dns_entry.get().strip()

        if not dns:
            self.append_output("Error:\nPlease enter a DNS IP address.")
            return

        try:
            ipaddress.ip_address(dns)
        except ValueError:
            self.append_output("Error:\nPlease enter a valid IP address.")
            return

        self.set_dns(dns)

    def restore_dns(self):
        self.run_command([self.dnschanger_path, "restore"], "Restoring DNS settings")

    def check_status(self):
        self.run_command([self.dnschanger_path, "status"], "Checking current DNS")


if __name__ == "__main__":
    # Check if running as root
    if os.geteuid() != 0:
        print("=" * 60)
        print("ERROR: Root privileges required!")
        print("=" * 60)
        print("\nThis GUI needs root access to modify DNS settings.")
        print("\nPlease run with sudo using one of these methods:")
        print("\n1. Preserve environment (recommended):")
        print("   sudo -E python3 gui/gui.py")
        print("\n2. If using virtual environment:")
        print("   sudo -E env PATH=$PATH python3 gui/gui.py")
        print("\n3. Allow X11 access for root:")
        print("   xhost +si:localuser:root")
        print("   sudo python3 gui/gui.py")
        print("=" * 60)
        sys.exit(1)

    # Check for DISPLAY environment variable (X11)
    if not os.environ.get('DISPLAY'):
        print("=" * 60)
        print("ERROR: No DISPLAY environment variable!")
        print("=" * 60)
        print("\nGUI cannot start without X11 display.")
        print("\nIf running with sudo, try:")
        print("   sudo -E python3 gui/gui.py")
        print("\nOr set DISPLAY manually:")
        print("   sudo DISPLAY=$DISPLAY python3 gui/gui.py")
        print("=" * 60)
        sys.exit(1)

    try:
        app = DNSChangerApp()
        app.mainloop()
    except Exception as e:
        print(f"\nError starting GUI: {e}")
        print("\nIf you see X11/display errors, try:")
        print("  xhost +si:localuser:root")
        print("  sudo -E python3 gui/gui.py")
        sys.exit(1)