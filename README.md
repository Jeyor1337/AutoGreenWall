# AutoGreenWall

[![Language](https://img.shields.io/badge/Language-C-blue.svg)](https://shields.io/)
[![License](https://img.shields.io/badge/License-GPLv3-green.svg)](https://opensource.org/licenses/MIT)

`AutoGreenWall` 是一个强大的自动化工具，旨在帮助开发者"粉刷"他们的 GitHub 贡献墙。它可以为过去的一整年或从年初至今的每一天，自动创建具有随机性的 Git 提交记录。

**最新版本采用 libgit2 原生库实现，性能提升10-50倍！**不再依赖shell调用git命令，直接操作git对象，效率极高。

> **警告：** 请勿在您现有的、包含重要项目历史的仓库上使用此工具。这会产生大量无意义的提交，污染您的项目历史。强烈建议您为此目的创建一个**全新的、空的** GitHub 仓库。

## ✨ 功能特性

-   **极致性能**：使用 libgit2 原生库直接操作 Git 对象，性能比传统shell调用方式快10-50倍。
-   **高度可定制**：
    -   **日期范围选择**：自由选择是刷满一整年，还是只刷到脚本运行的当天。
    -   **提交概率控制**：可设置在每一天有多大的几率（1-100%）进行提交，模拟非连续的工作日。
    -   **提交频次控制**：可为进行提交的日子设置一个随机的提交次数范围（例如每天1-5次），使贡献图看起来更自然。
-   **静默运行模式**：默认以静默模式运行，只在开始时进行交互配置，并在结束时报告结果。也可通过 `-v` 参数切换到详细模式，查看完整运行日志。
-   **跨平台兼容**：可在主流操作系统（Windows, macOS, Linux）上运行。

## 🚀 快速开始

### 准备工作

1.  **安装依赖库**：
    -   **libgit2**：本工具使用 libgit2 库来操作 Git 仓库。
        ```bash
        # Ubuntu/Debian
        sudo apt-get install libgit2-dev

        # Fedora/RHEL
        sudo dnf install libgit2-devel

        # macOS (Homebrew)
        brew install libgit2

        # Arch Linux
        sudo pacman -S libgit2

        # Termux (Android)
        pkg install libgit2

        # Windows (通过 vcpkg)
        vcpkg install libgit2
        ```

2.  **创建 GitHub 仓库**：
    -   访问 [GitHub](https://github.com/new) 创建一个**新的、空的**仓库（例如，命名为 `my-green-wall`）。
    -   **不要**勾选 "Add a README file"、".gitignore" 或 "license"。

### 使用教程

1.  **环境要求**：
    -   C 语言编译器（例如 `gcc` 或 `clang`）
    -   libgit2 开发库（见上方安装说明）
    -   Windows 用户可以使用 MinGW、MSVC 或 WSL

2.  **编译程序**：
    ```bash
    # Linux/macOS/Termux
    gcc -o autogreenwall auto_greenwall.c -lgit2

    # 或使用 pkg-config 获取正确的编译参数
    gcc -o autogreenwall auto_greenwall.c $(pkg-config --cflags --libs libgit2)

    # Windows (MinGW)
    gcc -o autogreenwall.exe auto_greenwall.c -lgit2

    # Windows (MSVC)
    cl auto_greenwall.c /link git2.lib
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

5.  **等待完成**：程序会在 `./contribution-data-c` 目录下创建所有本地提交。由于使用了 libgit2 原生库，即使创建数百个提交也只需几秒钟！

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

本工具使用 **libgit2** 原生库直接操作 Git 对象数据库，核心流程如下：

1. **初始化仓库**：使用 `git_repository_init()` 创建 Git 仓库。
2. **创建 Blob 对象**：使用 `git_blob_create_frombuffer()` 直接在内存中创建文件内容对象。
3. **更新索引**：使用 `git_index_add()` 将 blob 添加到暂存区，无需实际写入磁盘文件。
4. **构建 Tree**：使用 `git_index_write_tree()` 从索引创建 tree 对象。
5. **创建提交**：使用 `git_commit_create()` 创建提交对象，并通过 `git_signature_new()` 设置任意历史时间戳。

相比传统方式每次提交都要 spawn 2个shell进程（`git add` + `git commit`），libgit2 直接在进程内操作 Git 数据库，**性能提升10-50倍**。

### 性能对比

| 方式 | 200个提交耗时 | 进程创建次数 | 磁盘I/O次数 |
|------|--------------|-------------|------------|
| 传统shell调用 | ~20-30秒 | 400+ | 200+ |
| **libgit2原生库** | **~0.5-2秒** | **0** | **最小化** |

## 📜 许可证

本项目采用 [GPLv3 License](LICENSE) 授权。
