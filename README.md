# MC Fishing Bot (C++)

基于 **C++17 + Qt6 + OpenCV** 的 Minecraft 自动钓鱼助手，仅支持 Windows。

通过屏幕截图 + 模板匹配判断浮漂是否下沉，自动右键收竿，适合固定视角、固定分辨率的钓鱼场景。

## 功能

- 框选游戏画面中的水面区域作为监控范围
- 加载浮漂模板图片，用 OpenCV `matchTemplate` 判断是否上钩
- 每 200ms 截屏一次，匹配度超过阈值（默认 0.65）自动右键
- 4.5 秒冷却，60 秒未上钩自动重新抛竿
- 实时显示匹配度、进度条和等待时间
- Qt 置顶小窗口，操作简单

## 环境要求

- Windows 10 / 11
- CMake 3.16+
- 支持 C++17 的编译器（MSVC 2019+ 或 MinGW）
- Qt 6（Core、Gui、Widgets）
- OpenCV 4.x

## 构建

```bash
mkdir build
cd build

# MSVC 示例，Qt 路径按实际安装位置修改
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:/Qt/6.8.0/msvc2022_64" ^
  -DOpenCV_DIR="C:/opencv/build"

cmake --build . --config Release
```

生成的可执行文件为 `build/Release/MINECRAFT_FISHING_BOT_WIN.exe`。

如果 `find_package(OpenCV)` 失败，请设置环境变量 `OpenCV_DIR` 指向 OpenCV 的 `build` 目录。

## 使用

1. 启动 Minecraft，把浮漂保持在画面中相对固定的位置
2. 点击 **Select Area**，在屏幕上框选包含浮漂的水面区域
3. 点击 **Load Template**，选择 `resources/fish_template.png`，或自己截一张浮漂图片
4. 点击 **Start**，程序开始循环检测；点击 **Stop** 停止

## 项目结构

```
minecraft_fishing_bot_windows/
├── CMakeLists.txt
├── main.cpp              # 程序入口
├── main_window.cpp       # 主窗口、截屏、模板匹配、右键模拟
├── main_window.h
├── resources/
│   └── fish_template.png # 默认浮漂模板
└── build/                # 构建产物（未纳入仓库）
```

## 已知限制

- 模板匹配对分辨率、光影、视野缩放比较敏感，换环境需要重新截模板
- 游戏窗口需要保持前台可见，被遮挡时截到的是别的画面
- 目前没有配置持久化，每次启动需重新框选和加载模板
- 未做多显示器/DPI 缩放适配

## 免责声明

本项目仅供学习 Qt / OpenCV / Windows 截图 API 使用。在多人服务器使用自动化工具可能违反服务器规则，请自行确认并承担风险。
