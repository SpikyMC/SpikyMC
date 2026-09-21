<p align="center">
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/program_info/team.spiky.SpikyMC.logo-darkmode.svg">
  <source media="(prefers-color-scheme: light)" srcset="/program_info/team.spiky.SpikyMC.logo.svg">
  <img alt="SpikyMC" src="/program_info/team.spiky.SpikyMC.logo.svg" width="40%">
</picture>
</p>

<p align="center">
  SpikyMC is a custom launcher for Minecraft that allows you to easily manage multiple installations of Minecraft at once.<br />
  <br />SpikyMC is a <b>fork</b> of <a href="https://prismlauncher.org">Prism Launcher</a>, which itself is a fork of <a href="https://multimc.org">MultiMC</a>. It is <b>not</b> endorsed by, affiliated with, or supported by the Prism Launcher or MultiMC projects.
</p>

> [!NOTE]
> SpikyMC is an independent project. "SpikyMC", its logo and branding are the property of their respective owners and are used here only for informational purposes to indicate the origin of the code.

## Installation

- All downloads and installation instructions for SpikyMC can be found in the [Releases](https://github.com/SpikyTeam/SpikyMC/releases).
- The last build status can be found in the [GitHub Actions](https://github.com/SpikyTeam/SpikyMC/actions) tab (this also includes the pull request status).

### Development Builds

Please understand that these builds are not intended for most users. There may be bugs and other instabilities. You have been warned.

Development builds are available through:

- [GitHub Actions](https://github.com/SpikyTeam/SpikyMC/actions)

These have debug information in the binaries, so their file sizes are relatively larger.

Prebuilt Development builds are provided for **Linux**, **Windows** and **macOS**.
## Building

If you want to build SpikyMC yourself, check the [build instructions](https://prismlauncher.org/wiki/development/build-instructions)

## Credits & Acknowledgements

SpikyMC is based on the code and ideas of the following projects:

- **[Prism Launcher](https://prismlauncher.org)** — the launcher SpikyMC was forked from. Huge thanks to the Prism Launcher team and all contributors.
- **[MultiMC](https://multimc.org)** — the original launcher from which SpikyMC originated.

We are grateful to the Prism Launcher community for their work.

## Forking/Redistributing/Custom builds policy

You are free to fork, redistribute and provide custom builds of SpikyMC as long as you follow the terms of the [license](LICENSE) (this is a legal responsibility). If you made code changes rather than just packaging a custom build, please do the following as a basic courtesy:

- Make it clear that your fork is not SpikyMC and is not endorsed by or affiliated with the SpikyMC project (<https://github.com/SpikyTeam/SpikyMC>).
- Go through [CMakeLists.txt](CMakeLists.txt) and change SpikyMC's API keys to your own or set them to empty strings (`""`) to disable them (this way the program will still compile but the functionality requiring those keys will be disabled).

If you have any questions or want any clarification on the above conditions please make an issue and ask us.

If you are just building SpikyMC for your distribution, please make sure to set the `Launcher_BUILD_PLATFORM` to a slug representing your distribution. Examples are `archlinux`, `fedora` and `nixpkgs`.

Note that if you build this software without removing the provided API keys in [CMakeLists.txt](CMakeLists.txt) you are accepting the following terms and conditions:

- [Microsoft Identity Platform Terms of Use](https://docs.microsoft.com/en-us/legal/microsoft-identity-platform/terms-of-use)
- [CurseForge 3rd Party API Terms and Conditions](https://support.curseforge.com/en/support/solutions/articles/9000207405-curse-forge-3rd-party-api-terms-and-conditions)

If you do not agree with these terms and conditions, then remove the associated API keys from the [CMakeLists.txt](CMakeLists.txt) file by setting them to an empty string (`""`).

## License [![License](https://img.shields.io/github/license/SpikyTeam/SpikyMC?label=License&logo=gnu&color=C4282D)](LICENSE)

All launcher code is available under the GPL-3.0-only license.

The logo and related assets are under the CC BY-SA 4.0 license.
