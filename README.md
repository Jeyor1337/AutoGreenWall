# AutoGreenWall

[![Language](https://img.shields.io/badge/Language-C-blue.svg)](https://shields.io/)
[![License](https://img.shields.io/badge/License-GPLv3-green.svg)](https://opensource.org/licenses/MIT)

`AutoGreenWall` 是一个强大的自动化工具，旨在帮助开发者“粉刷”他们的 GitHub 贡献墙。它可以为过去的一整年或从年初至今的每一天，自动创建具有随机性的 Git 提交记录。

这个项目提供一个C语言实现版本，运行效率更高。

> **警告：** 请勿在您现有的、包含重要项目历史的仓库上使用此工具。这会产生大量无意义的提交，污染您的项目历史。强烈建议您为此目的创建一个**全新的、空的** GitHub 仓库。

## ✨ 功能特性

-   **高度可定制**：
    -   **日期范围选择**：自由选择是刷满一整年，还是只刷到脚本运行的当天。
    -   **提交概率控制**：可设置在每一天有多大的几率（1-100%）进行提交，模拟非连续的工作日。
    -   **提交频次控制**：可为进行提交的日子设置一个随机的提交次数范围（例如每天1-5次），使贡献图看起来更自然。
-   **静默运行模式**：默认以静默模式运行，只在开始时进行交互配置，并在结束时报告结果。也可通过 `-v` 参数切换到详细模式，查看完整运行日志。
-   **跨平台兼容**：可在主流操作系统（Windows, macOS, Linux）上运行。

## 🚀 快速开始

### 准备工作

1.  **安装 Git**：确保您的系统已安装 Git，并已将其添加到系统环境变量中。
2.  **创建 GitHub 仓库**：
    -   访问 [GitHub](https://github.com/new) 创建一个**新的、空的**仓库（例如，命名为 `my-green-wall`）。
    -   **不要**勾选 "Add a README file"、".gitignore" 或 "license"。

### 使用教程

1.  **环境要求**：确保您有 C 语言编译器（例如 `gcc`）。Windows 用户可以使用 MinGW 或 WSL。
2.  **编译程序**：
    ```bash
    # 编译命令 (在 Linux/macOS/MinGW 上)
    gcc -o autogreenwall commit_filler.c
    ```
3.  **运行程序**：
    -   **静默模式 (默认)**：
        ```bash
        ./autogreenwall
        ```
    -   **详细模式 (显示所有日志)**：
        ```bash
        ./autogreenwall -v
        # 或者
        ./autogreenwall --verbose
        ```
4.  **交互式配置**：根据提示输入您的配置。
5.  **等待完成**：程序会在 `./contribution-data-c` 目录下创建所有本地提交。

### 最后一步：推送至 GitHub

当脚本或程序执行完毕后，按照终端最后的提示，将所有本地提交推送到您的远程仓库。

```bash
# 1. 进入脚本生成的目录 (目录名取决于您使用的版本)
cd contribution-data

# 2. 关联您的远程 GitHub 仓库 (请将 URL 替换为您自己的)
git remote add origin https://github.com/your-username/my-green-wall.git

# 3. 确保主分支名为 'main'
git branch -M main

# 4. 推送所有本地提交
git push -u origin main
```

现在，访问您的 GitHub 个人资料页，稍等片刻，您将看到一堵漂亮的绿色贡献墙！

## ⚙️ 工作原理

本工具的核心原理是通过修改 Git 的环境变量来“欺骗”提交系统：

-   `GIT_AUTHOR_DATE`: 设置提交作者的时间戳。
-   `GIT_COMMITTER_DATE`: 设置提交操作本身的时间戳。

通过在每次执行 `git commit` 之前设置这两个变量为过去的某个特定时间，我们就可以创建出在任意历史日期发生的提交记录。

## 📜 许可证

本项目采用 [GPLv3 License](LICENSE) 授权。
