typedef struct {
	int fn;     // The function id in the Lua registry
	int delay;  // The delay in ms
	int repeat; // 1 = repeat, 0 = don't repeat
} Timer;

typedef struct {
	int fn;     // The function id in the Lua registry
	int data;   // Optional extra data in the Lua registry
} Callback;