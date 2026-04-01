# CarRenderFactory 架构文档

## 项目概述

汽车渲染工厂 (CarRenderFactory) 是一个基于 Unreal Engine 5.7 的汽车渲染和可视化项目。

---

## 1. 项目整体架构

```mermaid
graph TB
    subgraph "CarRenderFactory 项目架构"
        direction TB

        subgraph "构建系统"
            BT[CarRenderFactory.Target.cs<br/>游戏运行时目标]
            ET[CarRenderFactoryEditor.Target.cs<br/>编辑器目标]
        end

        subgraph "主模块 - CarRenderFactory"
            direction TB
            BC[CarRenderFactory.Build.cs<br/>模块依赖配置]
            MH[CarRenderFactory.h<br/>模块头文件]
            MC[CarRenderFactory.cpp<br/>模块实现]
        end

        subgraph "引擎模块"
            CORE[Core<br/>核心功能]
            CUO[CoreUObject<br/>对象系统]
            ENG[Engine<br/>引擎核心]
            INP[InputCore<br/>输入基础]
            EIN[EnhancedInput<br/>增强输入]
        end

        BT --> BC
        ET --> BC
        BC --> MH
        BC --> MC

        MH --> CORE
        MC --> CUO
        MC --> ENG
        MC --> INP
        MC --> EIN
    end

    style BT fill:#4a90d9,color:#fff
    style ET fill:#5cb85c,color:#fff
    style BC fill:#f0ad4e,color:#fff
    style MH fill:#5bc0de,color:#fff
    style MC fill:#5bc0de,color:#fff
```

---

## 2. 模块依赖关系

```mermaid
graph LR
    subgraph "CarRenderFactory 模块依赖"
        direction LR

        CRF[CarRenderFactory<br/>主游戏模块]

        subgraph "公共依赖 (Public)"
            P1[Core]
            P2[CoreUObject]
            P3[Engine]
            P4[InputCore]
            P5[EnhancedInput]
        end

        subgraph "可选依赖"
            O1[Slate / SlateCore<br/>UI框架]
            O2[OnlineSubsystem<br/>在线功能]
            O3[RenderCore<br/>渲染核心]
            O4[AIModule<br/>AI系统]
        end

        CRF -->|公共| P1
        CRF -->|公共| P2
        CRF -->|公共| P3
        CRF -->|公共| P4
        CRF -->|公共| P5

        CRF -.->|可选| O1
        CRF -.->|可选| O2
        CRF -.->|可选| O3
        CRF -.->|可选| O4
    end

    style CRF fill:#e74c3c,color:#fff
    style P1 fill:#3498db,color:#fff
    style P2 fill:#3498db,color:#fff
    style P3 fill:#3498db,color:#fff
    style P4 fill:#3498db,color:#fff
    style P5 fill:#3498db,color:#fff
    style O1 fill:#95a5a6,color:#fff
    style O2 fill:#95a5a6,color:#fff
    style O3 fill:#95a5a6,color:#fff
    style O4 fill:#95a5a6,color:#fff
```

---

## 3. 目录结构

```mermaid
graph TB
    subgraph "项目目录结构"
        direction TB

        ROOT[D:/UE/Project/CarRenderFactory/]

        ROOT --> SOURCE[Source/]

        SOURCE --> TARGET1[CarRenderFactory.Target.cs<br/>🎮 游戏目标]
        SOURCE --> TARGET2[CarRenderFactoryEditor.Target.cs<br/>🔧 编辑器目标]
        SOURCE --> AGENTS[AGENTS.md<br/>📝 架构文档]

        SOURCE --> MODULE[CarRenderFactory/]

        MODULE --> BUILD[CarRenderFactory.Build.cs<br/>⚙️ 构建配置]
        MODULE --> HEADER[CarRenderFactory.h<br/>📄 模块头文件]
        MODULE --> IMPL[CarRenderFactory.cpp<br⚙️ 模块实现]
        MODULE --> MODAGENT[AGENTS.md<br/>📝 模块文档]

        MODULE --> FUTURE[未来扩展...]
        FUTURE --> PUBLIC[Public/<br/>公开头文件]
        FUTURE --> PRIVATE[Private/<br/>私有实现]
        FUTURE --> CLASSES[Classes/<br/>蓝图类]
    end

    style ROOT fill:#2c3e50,color:#fff
    style SOURCE fill:#34495e,color:#fff
    style MODULE fill:#7f8c8d,color:#fff
    style TARGET1 fill:#4a90d9,color:#fff
    style TARGET2 fill:#5cb85c,color:#fff
    style BUILD fill:#f0ad4e,color:#fff
```

---

## 4. 构建流程

```mermaid
flowchart TD
    subgraph "UE5 构建流程"
        direction TB

        START([开始构建]) --> UBT[UnrealBuildTool]

        UBT --> PARSE{解析 .Target.cs}

        PARSE -->|Game| GTARGET[CarRenderFactory.Target.cs]
        PARSE -->|Editor| ETARGET[CarRenderFactoryEditor.Target.cs]

        GTARGET --> LOADMOD[加载模块配置]
        ETARGET --> LOADMOD

        LOADMOD --> BUILDCS[解析 CarRenderFactory.Build.cs]

        BUILDCS --> DEPS[解析依赖关系]

        DEPS --> COMPILE[编译 C++ 代码]

        COMPILE --> LINK[链接模块]

        LINK --> OUTPUT{输出类型}

        OUTPUT -->|Game| GAME[CarRenderFactory.exe<br/>游戏可执行文件]
        OUTPUT -->|Editor| EDITOR[UEEditor.exe<br/>编辑器可执行文件]

        GAME --> END([构建完成])
        EDITOR --> END
    end

    style START fill:#27ae60,color:#fff
    style END fill:#27ae60,color:#fff
    style UBT fill:#3498db,color:#fff
    style BUILDCS fill:#f39c12,color:#fff
    style COMPILE fill:#e74c3c,color:#fff
    style GAME fill:#4a90d9,color:#fff
    style EDITOR fill:#5cb85c,color:#fff
```

---

## 5. 模块生命周期

```mermaid
sequenceDiagram
    participant Engine as UE 引擎
    participant MM as ModuleManager
    participant Module as CarRenderFactory

    Note over Engine,Module: 编辑器启动流程

    Engine->>MM: 启动引擎
    MM->>Module: 加载模块 (LoadModule)
    activate Module

    Module->>Module: StartupModule()
    Note right of Module: 初始化资源<br/>注册控制台命令<br/>绑定委托

    MM-->>Engine: 模块就绪

    Note over Engine,Module: 运行时

    Engine->>Module: 游戏逻辑执行

    Note over Engine,Module: 关闭流程

    Engine->>MM: 关闭引擎
    MM->>Module: ShutdownModule()
    Note right of Module: 释放资源<br/>取消委托绑定

    Module-->>MM: 清理完成
    deactivate Module
```

---

## 6. 类继承结构 (规划)

```mermaid
classDiagram
    class IModuleInterface {
        <<interface>>
        +StartupModule()
        +ShutdownModule()
    }

    class FDefaultGameModuleImpl {
        +StartupModule()
        +ShutdownModule()
        +IsGameModule()
    }

    class FCarRenderFactoryModule {
        +StartupModule()
        +ShutdownModule()
        -自定义初始化逻辑
    }

    IModuleInterface <|.. FDefaultGameModuleImpl
    FDefaultGameModuleImpl <|-- FCarRenderFactoryModule

    note for FCarRenderFactoryModule "未来可扩展的\n自定义模块类"
```

---

## 7. 输入系统架构

```mermaid
flowchart LR
    subgraph "Enhanced Input 系统"
        direction TB

        subgraph "输入源"
            KB[键盘]
            MS[鼠标]
            GP[手柄]
        end

        subgraph "输入映射"
            IMC[Input Mapping Context<br/>输入映射上下文]
            IA[Input Action<br/>输入动作]
        end

        subgraph "处理层"
            MOD[Modifiers<br/>修饰器<br/>死区/缩放/平滑]
            TRG[Triggers<br/>触发器<br/>按下/长按/双击]
        end

        subgraph "游戏逻辑"
            PC[PlayerController]
            PAWN[Pawn/Character]
        end

        KB --> IMC
        MS --> IMC
        GP --> IMC

        IMC --> IA
        IA --> MOD
        MOD --> TRG
        TRG --> PC
        PC --> PAWN
    end

    style IMC fill:#9b59b6,color:#fff
    style IA fill:#9b59b6,color:#fff
    style MOD fill:#1abc9c,color:#fff
    style TRG fill:#1abc9c,color:#fff
```

---

## 8. 建议的功能模块划分

```mermaid
graph TB
    subgraph "CarRenderFactory 功能模块规划"
        direction TB

        CORE_MOD[Core/<br/>核心系统]

        subgraph "渲染系统"
            REN_MOD[Rendering/]
            REN_MOD --> MAT[材质系统]
            REN_MOD --> LIGHT[光照系统]
            REN_MOD --> POST[后处理]
        end

        subgraph "车辆系统"
            VEH_MOD[Vehicles/]
            VEH_MOD --> CAR[汽车模型]
            VEH_MOD --> PHYS[物理模拟]
            VEH_MOD --> CAM[相机系统]
        end

        subgraph "UI系统"
            UI_MOD[UI/]
            UI_MOD --> HUD[HUD显示]
            UI_MOD --> MENU[菜单界面]
        end

        subgraph "工具系统"
            TOOL_MOD[Tools/]
            TOOL_MOD --> CONFIG[配置工具]
            TOOL_MOD --> DEBUG[调试工具]
        end

        CORE_MOD --> REN_MOD
        CORE_MOD --> VEH_MOD
        CORE_MOD --> UI_MOD
        CORE_MOD --> TOOL_MOD
    end

    style CORE_MOD fill:#e74c3c,color:#fff
    style REN_MOD fill:#3498db,color:#fff
    style VEH_MOD fill:#2ecc71,color:#fff
    style UI_MOD fill:#9b59b6,color:#fff
    style TOOL_MOD fill:#f39c12,color:#fff
```

---

## 9. 数据流架构

```mermaid
flowchart TB
    subgraph "渲染数据流"
        direction TB

        INPUT[用户输入] --> CTRL[Controller]
        CTRL --> PAWN[Pawn/Vehicle]

        PAWN --> TRANSFORM[Transform 数据]
        TRANSFORM --> SCENE[Scene Component]

        SCENE --> PRIM[Primitive Component]
        PRIM --> RENDER[Render Proxy]

        RENDER --> RHI[RHI 命令]
        RHI --> GPU[GPU 渲染]

        subgraph "渲染管线"
            GPU --> DEPTH[深度Pass]
            DEPTH --> BASE[Base Pass]
            BASE --> LIGHTING[光照计算]
            LIGHTING --> SHADOW[阴影Pass]
            SHADOW --> POSTFX[后处理]
            POSTFX --> OUTPUT[最终输出]
        end
    end

    style INPUT fill:#27ae60,color:#fff
    style GPU fill:#e74c3c,color:#fff
    style OUTPUT fill:#3498db,color:#fff
```

---

## 10. 技术栈总览

```mermaid
mindmap
  root((CarRenderFactory))
    引擎
      Unreal Engine 5.7
      C++ 20
      Blueprint
    渲染
      DirectX 12
      Lumen GI
      Nanite
      Virtual Shadow Maps
    输入
      Enhanced Input
      Input Mapping Context
      Input Actions
    模块
      Core
      CoreUObject
      Engine
      EnhancedInput
    工具
      Rider for Unreal
      Visual Studio 2022
      UE Editor
```

---

## 快速导航

| 文档 | 路径 | 说明 |
|------|------|------|
| 根目录文档 | [Source/AGENTS.md](./AGENTS.md) | 项目整体说明 |
| 模块文档 | [CarRenderFactory/AGENTS.md](./CarRenderFactory/AGENTS.md) | 主模块详情 |

---

*文档生成时间: 2026-03-30*
