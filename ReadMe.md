
## 最后编辑时间

2023-09-17

## The Whole Process of Stereo Camera Calibration 

### devices：
mindvision(迈德威视)双目相机

彩色双目相机，型号为MV-MSU134GC2-T; 镜头：莱纳 4.3mm 

工业黑白相机，型号为MV-MSU134GM2-T; 镜头：莱纳 4.3mm （附加：850nm滤片）

### why？
`yaml`配置文件支持多种不同语言读取数据，为了利用其方便、易读、快速的性质，编写脚本存放在此仓库，对matlab下双目相机标定的保存数据（mat文件）进行脚本式转化。

Ans: 在双目标定工具箱里输出的只有.mat文件，所以需要一个脚本读取其中数据，考虑到最后用的是.yaml文件读取相机参数，故完成此步骤的转换。

### 标定流程

- step1: 使用mindvision演示程序采集标定图片(演示程序只能使用在windows和Linux(?)平台下,其他系统利用api接口根据所需语言自行编写采集程序)，设置图片大小(一般来说无需设置,相机拍下多少就是多少,设备的图像分辨率默认为1024x2560），保存在Pictures文件夹；
- step2: 运行脚本`split_pictures.py`，完成图片切割；
- step3: 使用MATLAB相机标定工具箱（Stereo Camera Calibrator，棋盘格大小为35mm）；如图设置：
![img.png](Calibration/Src/setupParams.png)
- step4: 标定完成后，在工作区导出stereoParams，运行`processStereoParams.m`，得到CameraParams.mat；
- step5: 运行脚本`writeYaml.py`，生成新的相机配置文件`camera_paras.yaml`

### FD-文件描述和处理流程

对主文件夹中各文件/脚本的文件说明:

处理流程：图片 -> `split_picture.py` -> matlab双目标定 ->  `processStereoParams.m` -> `writeYaml.py` -> 生成相机参数yaml文件，处理完成

``` text
├── Calibration                     # 标定处理主目录
│   ├── Output
│   │   ├── CameraParams.mat        # 脚本:processStereoParams.m 默认输出结果
│   │   ├── calibrationSession.mat  # (若存在),相机标定会话,包含标定过程的所有相机参数
│   │   └── camera_paras.yaml       # 生成的相机参数配置文件
│   ├── Script
│   │   ├── file_input_output.py    # File Input and Output using XML and YAML files
│   │   ├── processStereoParams.m   # 处理导出到Matlab工作区的stereoParams并输出.
│   │   ├── split_picture.py        # 分割双目图像
│   │   └── writeYaml.py            # 写入配置文件
│   └── Src
│       ├── Pictures                # 默认采集到的图片读取位置
│       ├── L                       # 默认分割后左相机的保存位置
│       ├── R                       # 默认分割后右相机的保存位置
│       └── setupParams.png
├── ReadMe.md
└── Tools                           # 开发demo,手册等目录
    ├── README.md
    ├── 开发demo
    │   ├── C++ builder
    │   ├── Python
    │   └── QT
    └── 产品使用手册
        ├── 彩色双目产品手册.pdf
        └── 黑白双目产品手册.pdf
```

### 特别说明
在生成当中的`yaml`文件当中，没有单独列出基线长度，基线长度为外参平移向量当中的第一个元素（取绝对值）

Ans：未使用到该参数，仅使用到内外侧、双目相机原点之间的旋转平移矩阵，畸变系数，基础矩阵F。





