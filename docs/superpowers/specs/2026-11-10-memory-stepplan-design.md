# Memory StepPlan Design

## Intent

Ganti struktur data lama yang memisahkan `Plan` dan `Step` menjadi struktur rute langsung:
setiap memory `M0..M15` memiliki `StepPlan 0..149`. Setiap memory disimpan sebagai satu file
LittleFS agar RAM ESP32-C3 tidak menampung seluruh 16 memory.

## Final behavior

```text
M0..M15
└── StepPlan 0..149
```

Setiap memory disimpan di:

```text
/memory_00.bin
...
/memory_15.bin
```

Robot mulai dari `StepPlan 0`, lalu berjalan berurutan. Robot berhenti sebelum menjalankan
`StepPlan[stopPlan]`. Dengan demikian:

- `stopPlan = 20` menjalankan `StepPlan 0..19`.
- `stopPlan = 150` menonaktifkan batas stop index.
- `Action::FINISH` selalu menghentikan robot saat step tersebut dieksekusi.
- Jika `stopPlan = 150`, robot tetap berhenti ketika mencapai `Action::FINISH`.

## StepPlan parameters

Setiap `StepPlan` menyimpan:

- `action`
- `sensor`
- `rightSpeed`
- `leftSpeed`
- `counterMode`
- `counter`
- `valueA`
- `counterA`
- `counterAMode`
- `valueB`
- `counterB`
- `counterBMode`
- `pidProfile`
- `followMode`
- `jumpEnabled`
- `jumpTargetMemory`
- `jumpTargetStep`

## Defaults

Semua 150 step pada memory baru/default diinisialisasi sebagai finish step:

```text
action        = FINISH
sensor        = IGN
rightSpeed    = 0
leftSpeed     = 0
counterMode   = NONE
counter       = 0
valueA        = 0
counterA      = 0
counterAMode  = NONE
valueB        = 0
counterB      = 0
counterBMode  = NONE
pidProfile    = 0
followMode    = NONE
jumpEnabled   = false
jumpTarget... = 0
```

## Auto-save

Perubahan parameter langsung menandai memory sebagai dirty. Sistem menyimpan memory setelah
300 ms sejak perubahan terakhir, menggunakan satu pending save per memory. Jika perubahan
berikutnya terjadi sebelum 300 ms, timer di-reset dan hanya satu penulisan dilakukan.

Save dipaksa segera saat:

- user berpindah memory;
- user keluar dari editor;
- user mengubah `stopPlan` lalu meninggalkan editor;
- sistem menerima perintah shutdown terkontrol.

File ditulis atomik melalui temporary file, flush, validasi checksum, lalu rename.

## Storage and RAM

RAM hanya memuat satu `MemoryProgram` aktif:

```cpp
MemoryProgram activeMemory;
```

Tidak boleh ada array RAM penuh seperti:

```cpp
MemoryProgram allMemory[16];
```

Kapasitas final:

```text
MEMORY_COUNT = 16
STEP_COUNT = 150
CHECKPOINT_COUNT = 30
CALIBRATION_PROFILE_COUNT = 5
```

## UI behavior

- Main UI tetap memakai asset `main1..main5` dari `uiMain.h`.
- Main2 memilih memory `0..15`; ENTER membuka editor memory tersebut.
- Editor memilih `StepPlan 0..149` dan parameter step aktif.
- `stopPlan` ditampilkan dan diedit pada level memory.
- Main3 ENTER menjalankan memory/checkpoint terpilih.
- Main5 BACK membuka Config.

## Goals

- Struktur data sesuai rute nyata: memory berisi 150 StepPlan.
- Auto-save tanpa menulis flash pada setiap repeat input.
- Default aman: robot berhenti pada semua step yang belum diatur.
- Persistence aman setelah reboot.
- RAM tetap terbatas untuk ESP32-C3.

## Non-goals

- Tidak mengubah pinout ESP32-C3 yang sudah ditetapkan.
- Tidak mengubah bitmap `uiMain.h`.
- Tidak membuat semua memory aktif di RAM.
- Tidak mempertahankan level `Plan` terpisah.

## Implementation calls

- `stopPlan` memakai rentang `0..150`; nilai `150` berarti disabled.
- Stop boundary diperiksa sebelum eksekusi step.
- `FINISH` diperiksa setelah step aktif dimuat/dieksekusi.
- Save coalescing memakai 300 ms.
- Satu file LittleFS menyimpan satu memory.

## Remaining implementation

Builder perlu memperbarui `models.h`, storage API dan format file, menu/editor state,
planner execution, default initialization, serta tests untuk boundary stop, FINISH,
auto-save coalescing, reboot reload, dan memory isolation.
