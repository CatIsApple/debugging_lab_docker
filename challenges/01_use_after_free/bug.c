#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Widget Widget; // struct 없이 사용가능하게 alias 재정의

typedef struct { // 64비트 기준으로 16Bytes의 크기를 가짐
    void (*render)(Widget *self); // VTable이라는 구조체 안에 render 함수포인터 선언 (해당 함수포인터가 가리키고 있는 함수는 Widget타입의 특정 데이터 주소를 매개변수로 받음.)
    void (*on_event)(Widget *self, int code); // VTable이라는 구조체 안에 on_event 함수포인터 선언 (해당 함수포인터가 가리키고 있는 함수는 Widget타입의 특정 데이터 주소와 int타입의 code라는 값을 매개변수로 받음.)
} VTable; // VTable이라는 구조체 타입 선언

struct Widget { // VTable이라는 구조체 선언
    const VTable *vtbl;// VTable이라는 구조체 타입을 가지고있는 vtbl이라는 주소를 Widget 구조체 안에 선언
    int id;// id이라는 int타입 데이터를 Widget 구조체 안에 선언
    int closed;// closed이라는 int타입 데이터를 Widget 구조체 안에 선언
    char label[24];// label이라는 char[24]타입 데이터를 Widget 구조체 안에 선언
}; // 64비트 기준으로 40Bytes의 크기를 가짐

#define MAX_WIDGETS 8 //MAX_WIDGETS라는 부분을 모두 8로 교체함. (컴파일시 교체됨.)
typedef struct {
    Widget *items[MAX_WIDGETS]; // items[8]이라는 widget 타입의 데이터 선언. (8Bytes 포인터를 8개 지정하여 64Bytes를 차지함.)
    int count; // count라는 int타입 데이터 선언.
} Screen; // Screen이라는 구조체를 선언 ( 64비트 기준으로 68Bytes의 크기를 가짐.)(하지만 Screen에서 가장 큰 8Bytes를 기준으로 8의 배수를 맞추기 위해 4Bytes 크기의 padding을 넣으며 실제론 72Bytes가 됨.)

/* ── 위젯 종류별 동작 ─────────────────────────────────────────── */
static void button_render(Widget *self) { // 반환하는 값이 없으며, 이 파일에서만 참조할수 있는 Button_render라는 함수를 선언 ( Widget타입의 특정 데이터 주소를 매개변수로 받음.)
    printf("  [Button #%d] \"%s\"\n", self->id, self->label); // Button부분을 터미널에 render(Print)하며 받은 매개변수 안에있는 ID과 label을 출력.
}
static void label_render(Widget *self) { // 반환하는 값이 없으며, 이 파일에서만 참조할수 있는 label_render라는 함수를 선언 ( Widget타입의 특정 데이터 주소를 매개변수로 받음.)
    printf("  Label #%d: %s\n", self->id, self->label);  // Label부분을 터미널에 render(Print)하며 받은 매개변수 안에있는 ID과 label을 출력.
}
static void dialog_render(Widget *self) { // 반환하는 값이 없으며, 이 파일에서만 참조할수 있는 dialog_render라는 함수를 선언 ( Widget타입의 특정 데이터 주소를 매개변수로 받음.)
    printf("  <<Dialog #%d>> %s\n", self->id, self->label); // Dialog부분을 터미널에 render(Print)하며 받은 매개변수 안에있는 ID과 label을 출력.
}

static void widget_noop_event(Widget *self, int code) { (void)self; (void)code; } // Widget_noop_event라는 함수를 선언 후 아무런 작업도 하지 않는 이벤트 함수로 선언. ( 컴파일러에게 해당 함수에서 받는 self와 code는 일부러 사용하지 않는것이다. 라고 선언하며 경고를 방지. )

static void dialog_on_event(Widget *self, int code); // dialog_on_event라는 함수를 선언. 하지만 실제 함수 내에 코드는 아래에서 선언하기에 DIALOG_VT[Line 42]에 주소 지정을 위해 임시로 선언.

static const VTable BUTTON_VT = { button_render, widget_noop_event }; // BUTTON_VT라는 데이터를 VTable이라는 타입으로 선언하며, button_render와 widget_noop_event 함수를 삽입. (여기서 widget_noop_event는 event가 필요없지만 타입으로 인해 삽입된 함수.)
static const VTable LABEL_VT  = { label_render,  widget_noop_event }; // LABEL_VT라는 데이터를 VTable이라는 타입으로 선언하며, label_render와 widget_noop_event 함수를 삽입. (여기서 widget_noop_event는 event가 필요없지만 타입으로 인해 삽입된 함수.)
static const VTable DIALOG_VT = { dialog_render, dialog_on_event  }; // DIALOG_VT라는 데이터를 VTable이라는 타입으로 선언하며, dialog_render와 dialog_on_event 함수를 삽입. (여기서 dialog_on_event는 임시로 선언된 함수이며, 하단에서 함수 내에서 실행할 코드를 선언함.)

static Widget *widget_new(const VTable *vt, int id, const char *label) {// Widget타입의 포인터 주소를 반환하는 함수를 선언 (바꿀수 없는 Vtable 타입의 vt라는 포인터 주소, int타입의 id, 바꿀수 없는 char타입의 label라는 포인터 주소를 매개변수로 받음.)
    
    Widget *w = malloc(sizeof *w);  // Widget 타입의 w라는 포인터변수(*)를 선언한다. 그리고 해당 포인터 변수에 malloc(sizeof *w)에서 return된 heap 메모리 주소를 넣는다.
    // 여기서 sizeof *w는 컴파일 과정에서 계산된 후 malloc(Bytes)로 교체되어 컴파일된다.
    // *w는 w의 실제 데이터을 호출하는 연산자이며, 컴파일 단계에서는 w의 데이터가 채워지지 않은 상태지만.
    // Widget이라는 구조체 타입의 크기를 보고 sizeof는 결정하기에 Widget = 40Bytes 즉 40Bytes를 말록에 대입하여 컴파일하게된다.
    // sizeof는 특정 값 또는 주소의 실제 크기를 보지 않고 타입을 보고 계산한다. (실제 메모리엔 타입정보가 존재하지 않기에 컴파일시 코드 명시된 타입을 보고 계산하게 된다.)

    if (!w) { perror("malloc"); exit(1); } // w가 없거나 값이 존재하지 않는다면 Error ("malloc")이라는 내용을 프린트하며, exit의 status 1을 넣어 프로그램을 종료한다. ( 여기서 w가 없다는 것은 malloc에서 공간을 할당할때 제대로 할당되지 않아 return 값이 없는 경우 또는 Out of Memory, 할당 제한 초과 등이 존재한다. )
    // Line 52의 if문으로 w가 없는 상황에 대한 방어 프로그래밍 = 예외 처리 한 코드이다.

    w->vtbl = vt;// w라는 주소가 가리키는 값 내부에 vtbl의 주소값을 vt라는 주소값으로 교체한다. (w->vtbl)(여기서 ->는 주소가 가리키는 값 내부에 특정 데이터를 가리킨다. 실제론 (*w).vtbl으로 해석할수있다.)
    w->id = id;// w라는 주소가 가리키는 값 내부에 id라는 int타입의 데이터 값을 id라는 값으로 교체한다.
    w->closed = 0;// w라는 주소가 가리키는 값 내부에 closed라는 int타입의 데이터 값을 0으로 교체한다.

    strncpy(w->label, label, sizeof(w->label) - 1);// w->label에 label 문자열의 sizeof(w->lable)-1 갯수만큼 문자열을 복사한다.
    // 여기서 sizeof(w->label)-1는 컴파일시 23으로 계산된다. 24 - 1
    // 그렇기 때문에 만약 24개가 넘는 문자열이 label로 들어오게된다 해도 23개만큼 짜른후 w->label 복사하게된다.
    // 이부분에서 문자열 길이를 강제로 잘라서 넣기때문에 실제로 24의 바이트 크기를 넘지 않으며, buffer overflow를 방지한다.

    w->label[sizeof(w->label) - 1] = '\0';// w->label의 문자열에서 마지막 자리를 \0으로 바꾼다.
    //실제 메모리에서 문자열을 읽을땐 문자열의 시작점부터 \0라는 문자열 마지막의 종료지점까지 읽도록 되어있다.
    //그렇기 때문에 23개까지만 복사한 이유는 마지막에 종료표시인 \0을 삽입하기 위함이다.

    return w; // 현재 malloc으로 할당한 w의 주소를 리턴하며, 함수를 종료한다.
} // 이 함수는 새로운 Widget을 만들고 그 새로운 Widget의 주소를 리턴하는 함수다.
// 하나씩 설명하자면 (Line 46) malloc을 통해 heap영역에 Widget 구조체 크기를 할당한다. 이를 통해 기본적으로 값을 넣을수있는 초기 템플릿을 만든셈이다.
// 그 이후 w-> 를 통해 heap영역에 만든 초기 템플릿에 Widget 구조를 기반으로 값을 하나씩 집어넣는다.
// 그렇게 값을 전부 집어넣은 새로운 Widget 데이터인 w의 주소를 return한다.

static void widget_destroy(Widget *w) {// 반환하는 값이 없으며, 이 파일에서만 참조할수 있는 widget_destroy라는 함수를 선언 (Widget타입인 포인터 주소를 w라는 매개변수로 받는다.)
    free(w);// 할당된 w를 해제한다.
} // w라는 Widget의 주소를 받고 free로 해제하는 함수다.
// C언어 표준상 w가 NULL일 때는 free()가 알아서 아무 동작도 안 하고 안전하게 넘어가므로 굳이 함수 내부에서 if (!w) 같은 예외 처리를 수동으로 해줄 필요가 없다.

/* ── Screen ──────────────────────────────────────────────────── */
static void screen_add(Screen *s, Widget *w) {
    if (s->count < MAX_WIDGETS) s->items[s->count++] = w;
}

static void screen_dispatch(Screen *s, int code) {
    for (int i = 0; i < s->count; i++) {
        Widget *w = s->items[i];
        w->vtbl->on_event(w, code);
    }
}

static void screen_render(Screen *s) {
    for (int i = 0; i < s->count; i++) {
        Widget *w = s->items[i];
        w->vtbl->render(w);      
    }
}

static void dialog_on_event(Widget *self, int code) {
    if (code == 1) {
        self->closed = 1;
        widget_destroy(self);   
    }
}

static char *app_build_status(const char *text) {
    char *msg = malloc(sizeof(Widget));   
    if (!msg) exit(1);

    memset(msg, 0xAB, sizeof(Widget));
    snprintf(msg, sizeof(Widget), "STATUS: %s", text);
    return msg;
}

int main(void) {
    Screen s = { .count = 0 };

    screen_add(&s, widget_new(&LABEL_VT,  10, "Welcome"));
    screen_add(&s, widget_new(&BUTTON_VT, 11, "OK"));
    screen_add(&s, widget_new(&DIALOG_VT, 12, "Are you sure?"));  /* items[2] */
    screen_add(&s, widget_new(&BUTTON_VT, 13, "Cancel"));

    printf("frame 1:\n");
    screen_render(&s);
    screen_dispatch(&s, 1);

    char *status = app_build_status("dialog closed");
    printf("%s\n", status);

    printf("frame 2:\n");
    screen_render(&s);           

    free(status);
    for (int i = 0; i < s.count; i++) free(s.items[i]);
    return 0;
}
