> [!IMPORTANT]
> This project is currently only developer stable
>

<div style="text-align:center">
    <h1>Ultimate WebAssembly Virtual Machine</h1>
    <a href="https://github.com/ULTE/uwvm2/actions?query=workflow%3ACI">
      <img src="https://img.shields.io/github/actions/workflow/status/ULTE/uwvm2/ci.yml?branch=master" alt="github-ci" />
    </a>
    <a href="LICENSE.md">
        <img src="https://img.shields.io/badge/License-Apache%202.0-green.svg" , alt="License" />
    </a>
    <a href="https://en.cppreference.com">
        <img src="https://img.shields.io/badge/language-c++26-blue.svg" ,alt="cppreference" />
    </a>
</div>

## Introduce
Ultimate WebAssembly Virtual Machine (Version 2), the abbreviation is uwvm2.

## Features
### WASM Feature Extensions
Most wasm standards are supported. See [feature.md](documents/features.md)

### Supports multiple platforms
Supports over 100 triplet platforms, including DOS series, POSIX series, Windows 9x series, Windows NT series, and the Host C Library Series. Supports interpretation execution (INT), just-in-time compilation (JIT), and tiered compilation (TC). See [support.md](documents/support.md) for details.

## How to build
