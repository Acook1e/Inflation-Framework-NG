ScriptName InflationFrameworkNative Hidden

; =========================================================================
;  InflationFramework Papyrus API
;
;  自定义 morphType 是一个 opaque ID。
;  由于 Papyrus 的 int 是有符号 32 位，自定义 morphType 可能显示为负数，这属于正常情况，
;  只需要把它原样传回本脚本的其他 API 即可。
;
;  MorphType 枚举值:
;    0 = Belly
;    1 = BellyMid
;    2 = BellyUnder
;    3 = BellyPregnancy
;    4 = Breasts
;    5 = Butt
; =========================================================================

int Function GetAPIVersion() native global

; 通过已注册的 slider 名称获取 morphType，未找到返回 -1
int Function GetMorphType(string morphName) native global

; 注册自定义 morph，返回 morphType，失败返回 -1
int Function RegisterInflation(string morphName, float minValue, float maxValue) native global

; 注册菜单显示文本，morphName 应与 RegisterInflation 使用的名称一致
Function RegisterLocalization(string morphName, string label, string desc) native global

; --- 按 MorphType 枚举操作 ---

; 获取 actor 某 morph 的当前值
float Function GetInflation(Actor akActor, int morphType) native global

; 设置 actor 某 morph 的绝对值 (立即应用)
Function SetInflation(Actor akActor, int morphType, float value) native global

; 增量修改 actor 某 morph (立即应用)
Function ModInflation(Actor akActor, int morphType, float value) native global

; --- 按 morph slider 名字操作 (直接操作 SKEE, 不走枚举) ---

float Function GetMorphByName(Actor akActor, string morphName) native global
Function SetMorphByName(Actor akActor, string morphName, float value) native global

; --- 应用 morph ---
Function ApplyMorphs(Actor akActor) native global

; --- 查询 ---

; 获取某 MorphType 的 min/max 范围
float Function GetMorphMinValue(int morphType) native global
float Function GetMorphMaxValue(int morphType) native global
Function SetMorphMinValue(int morphType, float value) native global
Function SetMorphMaxValue(int morphType, float value) native global

; 获取 MorphType 对应的 SKEE slider 名字
string Function GetMorphSliderName(int morphType) native global

; =========================================================================
;  便捷属性 (MorphType 枚举常量)
; =========================================================================

int Property kMorphBelly          = 0 AutoReadOnly
int Property kMorphBellyMid       = 1 AutoReadOnly
int Property kMorphBellyUnder     = 2 AutoReadOnly
int Property kMorphBellyPregnancy = 3 AutoReadOnly
int Property kMorphBreasts        = 4 AutoReadOnly
int Property kMorphButt           = 5 AutoReadOnly
int Property kInvalidMorphType    = -1 AutoReadOnly
