#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint8_t a = 0xFF;

    // a + 1 önce int'e yükseltilir (0x100), cast ile 8 bite kesilir: 0x00.
    // 6502'nin 8 bitlik registerları da tam olarak böyle "sarar".
    a = (uint8_t)(a + 1);

    printf("NES iskeleti calisiyor. 0xFF + 1 = 0x%02X\n", a);
    return 0;
}
