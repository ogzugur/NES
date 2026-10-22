# NES Emülatörü — Öğrenme Yol Planı

Amaç: NES donanımını parça parça anlayarak, sıfırdan çalışan bir emülatör yazmak.
Kural: Kara kutu yok. Her bileşeni önce okuyup anlıyoruz, kendi notumuzu yazıyoruz, sonra kodluyoruz ve bir test ile doğruluyoruz.

---

## Kararlar

- **Dil:** C (C17). Derleyici MSVC (Visual Studio Community 2026), build sistemi CMake.
  Rust bilinçli olarak sonraya bırakıldı: NES'i ve Rust'ı aynı anda öğrenmek, bir hatanın hangisinden kaynaklandığını ayırt etmeyi zorlaştırır. C versiyonu bittikten sonra Rust'a port etmek ayrı bir öğrenme projesi olabilir.
- **Grafik/ses:** Aşama 3'te seçilecek (SDL veya raylib).
- **Kod yazma:** Küçük parçalar halinde. Bazen kullanıcı yazar, bazen Claude küçük bir parça yazar, her parça birlikte incelenir. Tek seferde büyük kod yok.

---

## Çalışma yöntemi (her aşamada aynı döngü)

1. **Oku**: O aşamanın kaynaklarını oku (aşağıda listelendi).
2. **Not al**: `notes/` altına kendi cümlelerimizle kısa bir özet yaz (ör. `notes/02-cpu-addressing-modes.md`). Anlatamıyorsan henüz anlamamışsındır.
3. **Küçük deney**: Gerekirse küçük bir prototip ya da elle hesaplanan bir örnek yap.
4. **Kodla**: Sadece o aşamanın kapsamını kodla.
5. **Doğrula**: Test ROM'u ya da test verisi ile kanıtla. Testi geçmeden bir sonraki aşamaya geçme.

---

## NES'e kuşbakışı: neyi emüle edeceğiz?

```
                 ┌──────────────── Kartuş ────────────────┐
                 │  PRG-ROM (kod)   CHR-ROM/RAM (grafik)  │
                 │  Mapper (bank switching), opsiyonel RAM │
                 └──────┬───────────────────────┬─────────┘
                        │ CPU bus               │ PPU bus
┌───────────────────────┴───┐            ┌──────┴──────────────────┐
│ CPU: Ricoh 2A03           │  $2000-07  │ PPU: Ricoh 2C02         │
│  - 6502 çekirdeği         │◄──────────►│  - 256x240 görüntü      │
│    (decimal mode yok)     │  NMI ◄─────│  - Arka plan + sprite    │
│  - APU (ses, 5 kanal)     │            │  - 2KB VRAM, OAM, palet  │
│  - 2KB RAM, OAM DMA       │            └─────────────────────────┘
│  - Kontrolcü portları     │
└───────────────────────────┘
NTSC: CPU ~1.79 MHz, PPU = 3 × CPU saati. Her CPU döngüsünde PPU 3 "dot" ilerler.
Bir kare = 262 scanline × 341 dot.
```

İlk okuma: [Rodrigo Copetti — NES Architecture](https://www.copetti.org/writings/consoles/nes/) (tüm sistemi çok iyi ve görsel olarak anlatır).

---

## Aşamalar

### Aşama 0: Temel ve hazırlık (1-3 gün)
**Öğreneceğimiz:** NES'in parçaları ve birbirleriyle nasıl konuştukları; 6502 assembly'yi *kullanıcı* olarak tanımak.
- [Copetti NES makalesi](https://www.copetti.org/writings/consoles/nes/) baştan sona okunur.
- [Easy 6502](https://skilldrick.github.io/easy6502/): tarayıcıda 6502 assembly yazarak öğreten interaktif tutorial. Emülatörünü yazacağın işlemciyi önce programla.
- Proje iskeleti: `git init`, CMake + MSVC ile derlenen ilk C programı.

**Kontrol noktası:** Easy 6502'de küçük bir program (ör. ekranda piksel hareket ettiren döngü) yazabiliyor ve yukarıdaki diyagramı kendi sözlerimizle anlatabiliyoruz.

### Aşama 1: 6502 CPU (en büyük ve en öğretici aşama)
**Öğreneceğimiz:** Registerlar (A, X, Y, SP, PC, P), status flag'leri, 13 adresleme modu, stack, fetch-decode-execute döngüsü, döngü (cycle) sayımı, sayfa sınırı (page cross) cezası, kesmeler (NMI/IRQ/BRK/RESET vektörleri).
- Önce düz 64KB bir bellek dizisiyle çalışan, bus'tan bağımsız bir CPU.
- Önce resmi 151 opcode, sonra NES oyunlarında kullanılan "illegal" opcode'lar.
- Baştan **disassembler + trace log** yaz: `C000  4C F5 C5  JMP $C5F5   A:00 X:00 Y:00 P:24 SP:FD CYC:7` formatında. Bu bizim bir numaralı hata ayıklama aracımız olacak.

**Kaynaklar:**
- [NESdev Wiki — 6502 instructions](https://www.nesdev.org/wiki/6502_instructions), [Status flags](https://www.nesdev.org/wiki/Status_flags), [CPU interrupts](https://www.nesdev.org/wiki/CPU_interrupts), [Unofficial opcodes](https://www.nesdev.org/wiki/CPU_unofficial_opcodes)
- [Masswerk 6502 Instruction Set](https://masswerk.at/6502/6502_instruction_set.html) — opcode matrisi, flag ve döngü bilgisi
- [Masswerk — 6502 "Illegal" Opcodes Demystified](https://masswerk.at/nowgobang/2021/6502-illegal-opcodes)
- [Obelisk 6502 Reference](http://www.6502.org/users/obelisk/) — klasik referans
- [Visual 6502](http://www.visual6502.org/JSSim/) — gerçek çipin transistör seviyesinde simülasyonu (merak edenler için: "donanım gerçekte ne yapıyor?")

**Doğrulama:**
- [SingleStepTests/65x02](https://github.com/SingleStepTests/65x02) içindeki `nes6502` klasörü: her opcode için binlerce JSON test (başlangıç durumu → tek komut → bitiş durumu + bus aktivitesi). Birim test olarak çalıştırırız; hangi komutun nerede yanlış olduğunu tam olarak gösterir.

**Kontrol noktası:** `nes6502` JSON testlerinin tamamı geçiyor.

### Aşama 2: Bus, bellek haritası, kartuş (iNES) ve Mapper 0
**Öğreneceğimiz:** CPU adres uzayı ve **mirroring** (2KB RAM'in $0000-$1FFF'e 4 kez yansıması, PPU registerlarının $2000-$3FFF'e yansıması), memory-mapped I/O kavramı, ROM dosya formatı, mapper'ın ne olduğu.
- `.nes` dosyasını (iNES başlığı) okuyan loader.
- Mapper 0 (NROM): bank switching olmayan en basit kartuş.
- CPU'yu düz bellek yerine bus üzerinden konuşur hale getir.

**Kaynaklar:** [CPU memory map](https://www.nesdev.org/wiki/CPU_memory_map), [iNES](https://www.nesdev.org/wiki/INES), [Mapper](https://www.nesdev.org/wiki/Mapper), [NROM](https://www.nesdev.org/wiki/NROM)

**Doğrulama:** `nestest.nes` "otomasyon modu": PC=$C000'dan başlat, ekran olmadan çalıştır, trace log'umuzu referans `nestest.log` ile satır satır karşılaştır (bir script ile ilk farklı satırı bul). Dosyalar: [nestest.nes](http://www.qmtpro.com/~nes/misc/nestest.nes), [nestest.log](http://www.qmtpro.com/~nes/misc/nestest.log), [nestest.txt](http://www.qmtpro.com/~nes/misc/nestest.txt).

**Kontrol noktası:** nestest.log ile ~8991 satırın tamamı (illegal opcode bölümü dahil) eşleşiyor. Bu noktada CPU'muz "bitti" sayılır.

### Aşama 3: PPU bölüm 1, ilk görüntü
**Öğreneceğimiz:** PPU'nun kendi adres uzayı (pattern table, nametable, attribute table, palet), PPU registerları ($2000-$2007) ve neden "iki yazmalı" olduklarını, VBlank ve NMI, OAM DMA ($4014), tile tabanlı grafik (2 bitplane'li 8×8 tile'lar).
- İlk görsel adım: CHR-ROM'daki **pattern table'ları** ekrana çiz (bir debug görüntüleyici). CPU'ya bile gerek yok!
- Sonra: her karenin sonunda nametable'ı tek seferde çizen basit (kare tabanlı) renderer + sprite'lar.
- PPU zamanlaması: CPU döngüsü başına 3 PPU dot, scanline 241'de VBlank → NMI.

**Kaynaklar:** [PPU](https://www.nesdev.org/wiki/PPU), [PPU registers](https://www.nesdev.org/wiki/PPU_registers), [PPU memory map](https://www.nesdev.org/wiki/PPU_memory_map), [PPU pattern tables](https://www.nesdev.org/wiki/PPU_pattern_tables), [PPU nametables](https://www.nesdev.org/wiki/PPU_nametables), [PPU attribute tables](https://www.nesdev.org/wiki/PPU_attribute_tables), [PPU palettes](https://www.nesdev.org/wiki/PPU_palettes), [PPU OAM](https://www.nesdev.org/wiki/PPU_OAM)

**Kontrol noktası:** Donkey Kong / Balloon Fight / Ice Climber gibi NROM oyunlarının başlık ekranı görünüyor.

### Aşama 4: Kontrolcü
**Öğreneceğimiz:** $4016/$4017 üzerinden seri okunan shift register mantığı (strobe).
**Kaynak:** [Standard controller](https://www.nesdev.org/wiki/Standard_controller)
**Kontrol noktası:** Donkey Kong oynanabiliyor (Aşama 3'ün basit renderer'ıyla bile).

### Aşama 5: PPU bölüm 2, scanline/dot seviyesinde render
**Öğreneceğimiz:** PPU'nun gerçekte piksel piksel nasıl çalıştığı: 262×341 kare zamanlaması, arka plan fetch boru hattı (shift registerlar), "loopy" scroll registerları (`v`, `t`, `x`, `w`), oyun ortasında scroll değiştirme (ör. Super Mario'nun sabit durum çubuğu), sprite değerlendirme (scanline başına 8 sprite), **sprite 0 hit**, 8×16 sprite'lar.

**Kaynaklar:** [PPU rendering](https://www.nesdev.org/wiki/PPU_rendering) (kare zamanlama diyagramı burada), [PPU scrolling](https://www.nesdev.org/wiki/PPU_scrolling) (en önemli sayfa), [PPU sprite evaluation](https://www.nesdev.org/wiki/PPU_sprite_evaluation)

**Doğrulama:** [nes-test-roms](https://github.com/christopherpow/nes-test-roms) koleksiyonundan `ppu_vbl_nmi`, `sprite_hit_tests_2005.10.05`, `sprite_overflow_tests`, `blargg_ppu_tests_2005.09.15b`.
**Kontrol noktası:** Super Mario Bros. scroll ve durum çubuğuyla doğru oynanıyor.

### Aşama 6: APU (ses)
**Öğreneceğimiz:** 2 pulse (kare dalga), triangle, noise (LFSR), DMC kanalları; envelope, sweep, length counter; frame counter; kanalların karıştırılması (mixer); 1.79 MHz'den 44.1/48 kHz'e örnekleme ve ses tamponu ile emülatör hızının senkronu.

**Kaynaklar:** [APU](https://www.nesdev.org/wiki/APU) ve alt sayfaları (Pulse, Triangle, Noise, DMC, Frame Counter, Mixer)
**Doğrulama:** `apu_test`, `blargg_apu_2005.07.30` (nes-test-roms içinde).
**Kontrol noktası:** Super Mario müziği doğru çalıyor. Önce tek pulse kanalı, sonra diğerleri.

### Aşama 7: Mapper'lar (oyun kütüphanesini açmak)
**Öğreneceğimiz:** Bank switching, PRG/CHR bankaları, mirroring kontrolü, scanline sayacıyla IRQ.
Sırayla: **MMC1** (1: Zelda, Metroid) → **UxROM** (2: Mega Man, Castlevania) → **CNROM** (3) → **MMC3** (4: SMB3, Kirby; scanline IRQ içerir). Bu beşi (0-4) NES kütüphanesinin büyük çoğunluğunu kapsar.
**Kaynaklar:** [Mapper](https://www.nesdev.org/wiki/Mapper) sayfasından her mapper'ın kendi sayfası.
**Doğrulama:** `mmc3_test_2` vb. + gerçek oyunlar.

### Aşama 8: Cila ve ileri konular (isteğe bağlı)
Save state, pil destekli SRAM kaydı (Zelda), dahili debugger (bellek / nametable / OAM görüntüleyici), performans, PAL desteği, daha doğru zamanlama (dummy read/write, açık bus davranışı).

---

## Hata ayıklama stratejisi (baştan kuracağız)
- **Trace log**: Her komut için tek satır. Hata bulmanın %80'i budur.
- **Referans emülatör**: [Mesen](https://www.mesen.ca/). Çok güçlü bir debugger ve trace logger'ı var. Bizim log'umuzu onunkiyle karşılaştırırız. *Kodunu kopyalamak için değil, gerçeği gözlemlemek için.*
- **Görsel debug pencereleri**: pattern table, nametable, palet, OAM görüntüleyicileri (Aşama 3'te başlar).
- Test ROM'ları: [NESdev — Emulator tests](https://www.nesdev.org/wiki/Emulator_tests) listesi ve [nes-test-roms](https://github.com/christopherpow/nes-test-roms) koleksiyonu.

---

## Kaynak listesi

### Ana referans (her şeyin kaynağı)
- [NESdev Wiki](https://www.nesdev.org/wiki/Nesdev_Wiki): Topluluğun donanım üzerinde yıllarca yaptığı tersine mühendisliğin dokümantasyonu. "Datasheet" yerine bunu kullanacağız (Nintendo resmi datasheet yayınlamadı).
- [NESdev Forum](https://forums.nesdev.org/): Takıldığımız ince detaylar için.

### Tutorial / yazı dizileri (yönlendirici olarak, kopyalamak için değil)
- [javidx9 / OneLoneCoder — NES Emulator From Scratch](https://github.com/OneLoneCoder/olcNES): C++ YouTube serisi, C'ye en yakın referansımız. Özellikle PPU ve scroll anlatımı görsel olarak çok iyi.
- [bugzmanov — Writing NES Emulator in Rust](https://bugzmanov.github.io/nes_ebook/): Adım adım ebook (CPU → bus → kartuş → PPU → kontrolcü). Kod Rust ama kavram anlatımı için okunabilir.
- [Yi Zhang — Writing your own NES emulator](https://yizhang82.dev/nes-emu-overview): Kısa blog dizisi, iyi bir genel bakış.
- [EmuDevz](https://afska.itch.io/emudevz): Oyunlaştırılmış, JavaScript tabanlı emülatör yazma rehberi.

### CPU (6502)
- [Easy 6502](https://skilldrick.github.io/easy6502/), [Masswerk 6502 Instruction Set](https://masswerk.at/6502/6502_instruction_set.html), [Obelisk 6502](http://www.6502.org/users/obelisk/), [Visual 6502](http://www.visual6502.org/JSSim/), [SingleStepTests/65x02](https://github.com/SingleStepTests/65x02)

### Genel mimari
- [Copetti — NES Architecture](https://www.copetti.org/writings/consoles/nes/)

### Topluluk
- r/EmuDev (Reddit) ve EmuDev Discord sunucusu

---

## İlerleme

- [ ] Aşama 0: Temel ve hazırlık
- [ ] Aşama 1: 6502 CPU (SingleStepTests geçiyor)
- [ ] Aşama 2: Bus + iNES + NROM (nestest.log eşleşiyor)
- [ ] Aşama 3: PPU 1, ilk görüntü
- [ ] Aşama 4: Kontrolcü
- [ ] Aşama 5: PPU 2, scroll ve sprite 0 (SMB oynanıyor)
- [ ] Aşama 6: APU
- [ ] Aşama 7: Mapper 1-4
- [ ] Aşama 8: Cila
