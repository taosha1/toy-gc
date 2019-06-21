#include<stdio.h>
#include<stdlib.h>
#define MAXNUM 256

//基本类型、引用类型
typedef enum {
    OBJECT_INT,OBJECT_OBJ
}object_type;

typedef struct s_object{
    object_type type;
    unsigned char marked;

    struct s_object* next;

    union {
        int val;
        struct {
            struct s_object* ref;
        };
    };
}object;

typedef struct s_vm{
    object* stack[MAXNUM];
    object* head;

    int max_obj;
    int cur_obj;
    int size;
}VM;

VM* newVM(){
    VM* vm = malloc(sizeof(VM));
    (*vm).size = 0;
    vm->head = NULL;
    vm->max_obj = 8;
    vm->cur_obj = 0;
    return vm;
}

void gc(VM* vm);

object* newObject(VM* vm,object_type type){
    if(vm->cur_obj == vm->max_obj){
        printf("gc...\n");
        gc(vm);
    }
    object* o = malloc(sizeof(object));
    o->type = type;
    o->next=vm->head;
    vm->head = o;
    o->marked = 0;
    vm->cur_obj++;
    return o;
}

void push(VM* vm,object* o){
    if(vm->size>=MAXNUM){
        printf("stack overflow");
        exit(-1);
    }
    vm->stack[vm->size++] = o;
}

object* pop(VM *vm){
    if(vm->size <= 0){
        printf("stack underflow");
        exit(-1);
    }
    return vm->stack[--vm->size];
}

void pushINT(VM* vm,int val){
    object* o = newObject(vm,OBJECT_INT);
    o->val = val;
    push(vm,o);
}

object* pushOBJ(VM* vm){
    object* o = newObject(vm,OBJECT_OBJ);
    o->ref = pop(vm);
    push(vm,o);

    return o;
}

//o2为已分配在栈上的变量
//void pushOBJ(VM* vm,object* o,object* o2){
//    o->type = OBJECT_OBJ;
//    o->ref =  o2;
//    push(vm,o);
//}

void markAll(object* o){
    if(o->marked)
        return;
    o->marked = 1;
    if(o->type==OBJECT_OBJ){
        markAll(o->ref);
    }
}

void mark(VM* vm){
    for(int i=0;i<vm->size;i++){
        markAll(vm->stack[i]);
    }
}

void sweep(VM* vm){
    object** first = &vm->head;
    while(*first){
        if(!(*first)->marked){
            object* unreach = *first;
            *first = unreach->next;
            free(unreach);
            vm->cur_obj--;
        }else{
            (*first)->marked = 0;
            first = &(*first)->next;
        }
    }
}
//void sweep(VM* vm){
//    object* first = vm->head;
//    while(first){
//        if(!first->marked){
//            object* unreach = first;
//            first = first->next;
//            free(unreach);
//            vm->cur_obj--;
//        }else{
//            first->marked = 0;
//            first = first->next;
//        }
//    }
//}

void gc(VM* vm){
    int num = vm->cur_obj;
    //printf("line 133 :%d\n",num);
    mark(vm);
    sweep(vm);
    //printf("line 136 :%d\n",vm->cur_obj);

    vm->max_obj = vm->cur_obj *2;
    printf("\nCollected %d objects, %d remaining.\n", num - vm->cur_obj,
         vm->cur_obj);
}

void freeVM(VM* vm){
    vm->size = 0;
    gc(vm);
    free(vm->stack);
}

void test1() {
    printf("Test 1: Objects on stack are preserved.\n");
    VM* vm = newVM();
    pushINT(vm,1);
    pushINT(vm,2);

    if(vm->cur_obj!=2){
        printf( "Should have preserved objects.");
        exit(-1);
    }
    gc(vm);
    freeVM(vm);
}

void test2() {
    printf("Test 2: Unreached objects are collected.\n");
    VM* vm = newVM();
    pushINT(vm, 1);
    pushINT(vm, 2);
    pop(vm);
    pop(vm);

    gc(vm);
    if(vm->cur_obj!=0){
        printf("Should have collected objects.");
        exit(-1);
    }
    freeVM(vm);
}

void test3() {
    printf("Test 3: Reach nested objects.\n");
    VM* vm = newVM();
    pushINT(vm, 1);
    pushINT(vm, 2);
    pushOBJ(vm);
    pushINT(vm, 3);
    pushINT(vm, 4);
    pushOBJ(vm);
    pushOBJ(vm);

    gc(vm);
    if(vm->cur_obj!=7){
        printf("Should have reached objects.");
        exit(-1);
    }
    freeVM(vm);
}

void test4() {
    printf("Test 4: Handle cycles.\n");
    VM* vm = newVM();
    pushINT(vm, 1);
    pushOBJ(vm);
    object* a = pushOBJ(vm);
    pushINT(vm, 3);
    pushINT(vm, 4);
    object* b = pushOBJ(vm);

    /* Set up a cycle, and also make 3 and 3 unreachable and collectible. */
    a->ref = b;
    b->ref = a;
    //printf("curObjNum : %d,maxObjNum:%d \n",vm->cur_obj,vm->max_obj);
    gc(vm);

    if(vm->cur_obj!=3){
        printf("Should have collected objects.");
    }
    freeVM(vm);
}

void perfTest() {
  printf("Performance Test.\n");
  VM* vm = newVM();

  for (int i = 0; i < 200; i++) {
    for (int j = 0; j < 5; j++) {
      pushINT(vm, i);
    }

    for (int k = 0; k < 5; k++) {
      pop(vm);
    }
  }
  freeVM(vm);
}

int main(){
 //   test1();
 //   test2();
//    test3();
//   test4();
    perfTest();
    return 0;
}
