/*
 * 选择排序
 *
 * */

#include <stdlib.h>
#include <stdio.h>

#define swap(x,y) {int temp = x; x = y; y = temp; }

//两层循环实现
void selectsort1(int *arr, int len)
{
     for(int i = 0; i < len; i++)
     {
        int min_index = i;
        for(int j = i; j < len; j++)
        {
            if(arr[j] < arr[min_index])
                min_index = j;
        }
        swap(arr[i], arr[min_index])
     }
}

//递归的方式实现
void selectsort2(int *arr,int start, int len)
{
    if(start >= len)
        return;

    int min_index = start;
    for(int j = start; j < len; j++)
    {
        if(arr[j] < arr[min_index])
            min_index = j;
    }
    swap(arr[start], arr[min_index]);
    selectsort2(arr, ++start, len);
}


int main(int argc, char *argv[])
{
    int arr[] = {44,3,38,5,47,15,36,26,27,2,46,4,19,50,48};
    int len = sizeof(arr)/sizeof(int);

    //selectsort2(arr, 0, len);
    selectsort1(arr, len);

    for(int i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");

    return 0;
}
