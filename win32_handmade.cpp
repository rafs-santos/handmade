#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static 
#define global_variable static 

struct win32_offscreen_buffer {
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

struct win32_window_dimension {
    int width;
    int height;
};

global_variable bool running;
global_variable win32_offscreen_buffer global_back_buffer;


internal win32_window_dimension win32_get_window_dimension(HWND window)
{
    win32_window_dimension ret;

    RECT window_rect;
    GetClientRect(window, &window_rect);
    ret.height = window_rect.bottom - window_rect.top;
    ret.width = window_rect.right - window_rect.left;

    return ret;
}
internal void render_weird_gradient(win32_offscreen_buffer *buffer, int x_offset, int y_offset)
{
    int width = buffer->width;
    int height = buffer->height;

    uint8_t *row = (uint8_t *)buffer->memory;
    for (int y = 0; y < height; y++) {
        uint32_t *pixel = (uint32_t *)row;
        for (int x = 0; x < width; x++) {
            // pixel on memory RR GG BB XX
            uint8_t blue = (x + x_offset);
            uint8_t green = (y + y_offset);

            *pixel++ = ((green << 8) | blue);
        }
        row += buffer->pitch;
    }
}

internal void win32_resize_dib_section(win32_offscreen_buffer *buffer, int width, int height)
{
    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;

    buffer->info.bmiHeader.biSize = sizeof(BITMAPINFO);
    buffer->info.bmiHeader.biWidth = width;
    buffer->info.bmiHeader.biHeight = -height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    buffer->bytes_per_pixel = 4;
    int bitmap_memory_size = (width * height) * buffer->bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT | MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = buffer->width * buffer->bytes_per_pixel;
   
}
internal void win32_update_window(win32_offscreen_buffer *buffer, HDC device_context, int window_width, int window_height)
{
    int Result = StretchDIBits(
        device_context,
        //X, Y, Width, Height,
        0, 0, window_width, window_height,
        0, 0, buffer->width, buffer->height,
        buffer->memory,
        &buffer->info,
        DIB_RGB_COLORS, SRCCOPY
    );
}

LRESULT CALLBACK win32_main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT Result = 0;
    switch (message) {
        case WM_SIZE:
        {
            win32_window_dimension window_dimension = win32_get_window_dimension(window);
            win32_resize_dib_section(&global_back_buffer, window_dimension.width, window_dimension.height);
        } break;

        case WM_CLOSE:
        {
            //TODO: Handle this with a message to the user
            running = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugString("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY:
        {
            // Handle this with a message to the user
            running = false;
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            win32_window_dimension window_dimension = win32_get_window_dimension(window);

            win32_update_window(&global_back_buffer, device_context, window_dimension.width, window_dimension.height);
            EndPaint(window, &paint);
        } break;

        default:
        {
            //OutputDebugString("default\n");
            Result = DefWindowProc(window, message, wparam, lparam);
        } break;
    }
    return Result;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine, int ShowCode)
{
    //TODO: testes
    WNDCLASS window_class = {};
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = win32_main_window_callback;
    window_class.hInstance = Instance;
    window_class.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&window_class)) {
        HWND window = CreateWindowExA(
            0,                                  // Optional window styles.
            window_class.lpszClassName,          // Window class
            "Handmade Hero",                    // Window text
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,   // Window style
            // Size and position
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,       // Parent window    
            NULL,       // Menu
            Instance,  // Instance handle
            NULL        // Additional application data
        );
        if (window) {
            HDC device_context = GetDC(window);

            int x_offset = 0;
            int y_offset = 0;

            MSG message;
            running = true;

            while (running) {
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT) {
                        running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
                render_weird_gradient(&global_back_buffer, x_offset, y_offset);

                win32_window_dimension window_dimension = win32_get_window_dimension(window);

                win32_update_window(&global_back_buffer, device_context, window_dimension.width, window_dimension.height);

                ReleaseDC(window, device_context);

                ++x_offset;
            }
        } else {
            //TODO: Logging
        }
    } else {
        //TODO: log error
    }

    return 0;
}
