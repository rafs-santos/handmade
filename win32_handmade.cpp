#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define internal static
#define local_persist static 
#define global_variable static 

typedef int32_t bool32;

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

global_variable bool32 global_running;
global_variable win32_offscreen_buffer global_back_buffer = {};

/*
 * NOTE: With compress definition below, when we need to change parameters just do in #define 
    NOTE: Try to explain the definition below to call function in xinput.lib without linking with xinput.lib
    #define X_INPUT_GET_STATE(name) is a macro and will define DWORD WINAPI name(...); with name you pass in it

    now we typedef the macro:
    typedef X_INPUT_GET_STATE(x_input_get_state);                                   this line produce the line below
    typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* pState);

    We create a default function stub with return ERROR_DEVICE_NOT_CONNECTED.
    We define global variable pointer to type x_input_get_state and with stub function

    We define XinputGetState to our pointer to call the function with the same name on xinput.h file

*/

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)

typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(x_input_get_state_stub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}

typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(x_input_set_state_stub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}

global_variable x_input_get_state *XInputGetState_ = x_input_get_state_stub;
global_variable x_input_set_state *XInputSetState_ = x_input_set_state_stub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void win32_load_input(void)
{
    HMODULE x_input_library = LoadLibraryA("xinput1_3.dll");
    if (x_input_library) {
        XInputGetState = (x_input_get_state *)GetProcAddress(x_input_library, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(x_input_library, "XInputSetState");
    }
}


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
internal void win32_display_buffer_in_window(win32_offscreen_buffer *buffer, HDC device_context, int window_width, int window_height)
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
    LRESULT ret = 0;
    switch (message) {
        case WM_SIZE:
        {
        } break;

        case WM_CLOSE:
        {
            //TODO: Handle this with a message to the user
            global_running = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugString("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY:
        {
            // Handle this with a message to the user
            global_running = false;
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32_t vk_code = wparam;
            bool was_down  = ( (lparam & (1 << 30)) != 0);
            bool is_down  = ( (lparam & (1 << 31)) == 0);
            if (was_down != is_down) {
                if (vk_code == 'W') {

                } else if(vk_code == 'A') {

                } else if (vk_code == 'S') {

                } else if (vk_code == 'D') {

                } else if (vk_code == 'Q') {

                } else if (vk_code == 'E') {

                } else if (vk_code == VK_UP) {

                } else if (vk_code == VK_LEFT) {

                } else if (vk_code == VK_DOWN) {

                } else if (vk_code == VK_RIGHT) {

                } else if (vk_code == VK_ESCAPE) {
                    if (is_down) {
                        OutputDebugStringA("IsDown\n");
                    }

                    if (was_down){
                        OutputDebugStringA("WasDown\n");
                    }
                }
            }
            bool32 alt_key_was_down = ((lparam & (1 << 29)));
            if (vk_code == VK_F4 && alt_key_was_down) {
                global_running = false;
            }
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            win32_window_dimension window_dimension = win32_get_window_dimension(window);

            win32_display_buffer_in_window(&global_back_buffer, device_context, window_dimension.width, window_dimension.height);
            EndPaint(window, &paint);
        } break;

        default:
        {
            //OutputDebugString("default\n");
            ret = DefWindowProc(window, message, wparam, lparam);
        } break;
    }
    return ret;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine, int ShowCode)
{
    win32_load_input();

    //TODO: testes
    WNDCLASSA window_class = {};

    win32_resize_dib_section(&global_back_buffer, 1280, 720);

    window_class.style = CS_HREDRAW | CS_VREDRAW;
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
            global_running = true;

            while (global_running) {
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if (message.message == WM_QUIT) {
                        global_running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                //TODO: Should we poll this more frequently
                for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++) {
                    XINPUT_STATE controller_state;
                    if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS) {
                        //NOTE: Controller is connected
                        //TODO: controller_state.dwPacketNumber increment rapidly?
                        XINPUT_GAMEPAD *pad = &controller_state.Gamepad;    //NOTE: just to make easy call gamepad (syntax only)
                        
                        bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        
                        bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
                        bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

                        bool a_button = (pad->wButtons & XINPUT_GAMEPAD_A);
                        bool b_button = (pad->wButtons & XINPUT_GAMEPAD_B);
                        bool x_button = (pad->wButtons & XINPUT_GAMEPAD_X);
                        bool y_button = (pad->wButtons & XINPUT_GAMEPAD_Y);
                        
                        int16_t stick_x = pad->sThumbLX;
                        int16_t stick_y = pad->sThumbLY;
                        
                        if (a_button) {
                            y_offset += 2;
                        }

                    } else {
                        //NOTE: Controller is not connected
                    }
                }
                
                render_weird_gradient(&global_back_buffer, x_offset, y_offset);

                win32_window_dimension window_dimension = win32_get_window_dimension(window);
                win32_display_buffer_in_window(&global_back_buffer, device_context,
                                                window_dimension.width, window_dimension.height);

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
