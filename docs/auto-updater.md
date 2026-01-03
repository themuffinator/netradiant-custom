# Auto Updater

This document explains how the in-app updater works and what to expect when you use it.

## Where to check for updates

- Use Help -> Check for VibeRadiant update for a manual check.
- Automatic checks run at startup (once per 24 hours) if enabled.
- Preferences -> Settings -> Updates lets you toggle automatic checks and "Include prerelease builds".

The updater reads `update.json` from the latest GitHub release. "Latest Build"
artifacts do not include this manifest, so they are not compatible with auto-update.

## Supported packages

The auto-updater only works with the official release packages:
- Windows: the zip package from GitHub Releases.
- Linux: the AppImage package from GitHub Releases.

Other platforms and packaging formats are not supported.

## What happens during an update

1. VibeRadiant downloads the update to a temporary folder.
2. The file is verified using the SHA-256 hash from the release manifest.
3. You are prompted to save any modified work.
4. The app closes, applies the update, and relaunches.

## Windows details

- The zip is expanded into the install directory.
- The install directory must be writable. Installs under protected locations
  (for example Program Files) may require running with elevated permissions
  or moving the install to a writable path.

## Linux details

- Auto-update requires the AppImage build and only works when running the AppImage.
- The downloaded AppImage replaces the existing one, so the file location must be writable.

## If an update fails

- Download the latest package from the GitHub Releases page and replace the existing install.
- If updates repeatedly fail, check that your install path is writable.
