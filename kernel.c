#include <stdint.h>

#define VIDEO_MEMORY 0xb8000
#define WHITE_ON_BLACK 0x07

void kmain(void) {
    char *video_memory = (char *)VIDEO_MEMORY;
    const char *message = "Hello, World!";
    int i;

    // Clear the screen
    for (i = 0; i < 80 * 25 * 2; i++) {
        video_memory[i] = 0;
    }

    // Write message to video memory
    for (i = 0; message[i] != '\0'; i++) {
        video_memory[i * 2] = message[i];
        video_memory[i * 2 + 1] = WHITE_ON_BLACK;
    }
    
    // Loop forever
    while (1);
}
