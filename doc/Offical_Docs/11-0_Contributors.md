# 开发者贡献

- [贡献代码流程](#1)
    - [创建个人仓库](#1.1)
    - [本地克隆仓库和分支](#1.2)
    - [提交代码](#1.3)
    - [通过 CI 验证](#1.4)
    - [Code Review](#1.5)
    - [代码合入](#1.6)
- [致谢开发者](#2) 

<a name="1"></a>

## 贡献代码流程

Paddle Serving 使用 Git 分支模式。通常，按以下步骤贡献代码：

<a name="1.1"></a>

**一.创建个人仓库**

Paddle Serving 社区一直在快速发展，每个人都写到官方回购中是没有意义的。所以，请先 `fork` 出个人仓库，并提交 `Pull Requests`。`fork` 个人仓库，只需前往 [Serving](https://github.com/PaddlePaddle/Serving) 页面并单击右上角 ["Fork"](https://github.com/PaddlePaddle/Serving/fork)。

<a name="1.2"></a>

**二.本地克隆仓库和分支**

创建个人仓库后，`clone` 个人仓库到本地计算机，默认创建本地 `develop` 分支。
```bash
git clone https://github.com/your-github-account/Serving
```

<a name="1.3"></a>

**三.提交代码**

本地修改代码并验证后，准备提交代码。在提交代码前请安装 [`pre-commit`](http://pre-commit.com/)、cpplint 和 pylint。
```bash
pip3 install pre-commit
pre-commit install

pip3 install cpplint pylint
```
在提交代码时，会进行代码格式检查和修正，待所有检查都通过后，方可提交。
```shell
   $  git commit
   CRLF end-lines remover...............................(no files to check)Skipped
   yapf.....................................................................Passed
   Check for added large files..............................................Passed
   Check for merge conflicts................................................Passed
   Check for broken symlinks................................................Passed
   Detect Private Key...................................(no files to check)Skipped
   Fix End of Files.........................................................Passed
   clang-format.............................................................Passed
   cpplint..................................................................Passed
   pylint...................................................................Passed
   copyright_checker........................................................Passed
   [my-cool-stuff c703c041] add test file
    1 file changed, 0 insertions(+), 0 deletions(-)
    create mode 100644 233
```

运行代码提交命令，提交到个人仓库，再通过 Github 页面创建一个 `pull request` 提交到 Paddel Serving 主仓库。
```bash
git push origin develop
```

<a name="1.4"></a>

**四.通过 CI 验证**

所有提交到 Paddle Serving 主仓库的 `pull request` 都会运行 `py36`、`py38`、`py39`的所有 CI 测试用例。全部通过后才能合入。

<a name="1.5"></a>

**五.Code Review**

所有提交的代码要经过管理员的评审，至少通过2人评审后方可合入。

<a name="1.6"></a>

**六.代码合入**

待通过全部 CI 验证，并且完成 Code Review 和修改后，由仓库管理员合入代码。

<a name="2"></a>

## 致谢开发者

- 感谢 [@loveululu](https://github.com/loveululu) 提供 Cube python API
- 感谢 [@EtachGu](https://github.com/EtachGu) 更新 docker 使用命令
- 感谢 [@BeyondYourself](https://github.com/BeyondYourself) 提供grpc教程，更新FAQ教程，整理文件目录。
- 感谢 [@mcl-stone](https://github.com/mcl-stone) 提供faster rcnn benchmark脚本
- 感谢 [@cg82616424](https://github.com/cg82616424) 提供unet benchmark脚本和修改部分注释错误
- 感谢 [@cuicheng01](https://github.com/cuicheng01) 提供PaddleClas的11个模型
- 感谢 [@Jiaqi Liu](https://github.com/LiuChiachi) 新增list[str]类型输入的预测支持
- 感谢 [@Bin Lu](https://github.com/Intsigstephon) 提供PP-Shitu C++模型示例
