# CS2 build 14197 兼容补丁

CS2 1.41.9.7（build 14197，server 2000943）移除了 `INetworkMessages::DeallocateNetMessageAbstract`，
并把 `INetworkMessageInternal` / `NetMessageInfo_t` 改成非虚表结构（ABI 变化）。本仓库锁定的子模块版本
早于这次改动，直接构建出来的插件在新服务端上会无声失效/崩溃，所以全新 checkout 后需要先打这两个补丁。

| 补丁 | 对应上游 | 作用 |
|---|---|---|
| `cs2-14197-s2sdk-netmessage.patch` | [alliedmodders/s2sdk#124](https://github.com/alliedmodders/s2sdk/pull/124) | CS2 14197 的 net message 句柄/接口布局 |
| `cs2-14197-mm-utils-deallocate.patch` | [FemboyKZ/mm-utils#1](https://github.com/FemboyKZ/mm-utils/pull/1) | 3 处 `DeallocateNetMessageAbstract` → `delete pData` |

> 上游仍未合并，这两份补丁是从 PR 分支 cherry-pick/应用后的 diff，等价内容。

## 1. 应用补丁

```bash
# s2sdk：仓库固定 a9863cf0，应用后提交（等价 cherry-pick alliedmodders/s2sdk#124）
cd s2sdk
git apply ../patches/cs2-14197-s2sdk-netmessage.patch
git add -A
git commit -m "Update net message handles for CS2 14197"

# mm-utils：仓库固定 ec4390a，应用即可（等价 FemboyKZ/mm-utils#1）
cd ../mm-utils
git apply ../patches/cs2-14197-mm-utils-deallocate.patch
```

Windows PowerShell 下把 `cd s2sdk` 换成 `cd s2sdk` 同样可用（`git apply` 路径分隔符无影响）。

## 2. 构建

前置：Python 3、[AMBuild](https://github.com/alliedmodders/ambuild)（`ambuild` 在 PATH 中）、MSVC。

```bash
# 本仓库 plugin-metadata.json 的版本号来自 git describe，先拉取上游 tag，否则显示 0.0.0
git fetch https://github.com/FemboyKZ/mm-cs2rockthevote "refs/tags/*:refs/tags/*"

mkdir -p build && cd build
python ../configure.py --enable-optimize
ambuild
```

**Windows 必读：**
- 源码里有 UTF-8 注释，MSVC 默认代码页 936 会把下一行吞进注释（报 `C2039: htmlSectionFormat` 之类）。
  构建前设置 `set CL=/utf-8`。
- 仓库路径不要含中文（`插件\` 这类目录会让 AMBuild/protoc 的路径编码坏掉）；用 ASCII 路径构建，
  或建一个 ASCII junction 指过去。
- 产物：`build/package/addons/cs2rockthevote/bin/win64/cs2rockthevote.dll`

参考产物（本机构建，CS2 14197）：2,547,200 字节，sha256
`2D21BAF44372A1E3C76A166DC39D2CA98D5A8992EC9DA26F9C0ED3644AA82D69`。

## 3. 部署

把 `build/package/addons/cs2rockthevote/bin/win64/cs2rockthevote.dll` 覆盖到
`<game>/csgo/addons/cs2rockthevote/bin/win64/`，并确认 `<game>/csgo/addons/metamod/cs2rockthevote.vdf`
存在（没有被改名禁用）。
