/*
 *  冒泡排序
 *  
 * */

#include <stdlib.h>
#include <stdio.h>


//采用两层循环的方式实现
void bubblesort1(int *arr, int len) 
{
    for(int i = 1; i < len; i++)
    {
        for(int j = 0; j < len - i; j++)
        {
            int temp = arr[j];
            if(arr[j] > arr[j+1])
            {
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}


//采用递归的方式实现
void bubblesort2(int *arr, int len) 
{
    if(len < 2)
        return;
    for(int j = 0; j < len - 1; j++)
    {
        int temp = arr[j];
        if(arr[j] > arr[j+1])
        {
            arr[j] = arr[j+1];
            arr[j+1] = temp;
        }
    }
    bubblesort2(arr, --len);
}

int main(int argc, char *argv[])
{
    int arr[] = {44,3,38,5,47,15,36,26,27,2,46,4,19,50,48};
    int len = sizeof(arr)/sizeof(int);

//    bubblesort1(arr, len);
    bubblesort2(arr, len);

    for(int i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");

    return 0;
}
