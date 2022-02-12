/*
 * 希尔排序
 *
 */

#include <stdlib.h>
#include <stdio.h>

void groupsort(int *arr, int len, int start, int step)
{
    int j;
    for(int i = start + step; i < len; i+=step)
    {
        int temp = arr[i];
        for(j = i - step; j >= 0; j -= step)
        {
            if(temp > arr[j])
                break;
            arr[j+step] = arr[j];
        }
        arr[j+step] = temp;
    }
}

void shellsort(int *arr, int len)
{
    for(int step = len/2; step > 0; step = step/2)
    {
        for(int i = 0;i < step; i++)
            groupsort(arr, len, i, step);
    }
}


int main(int argc, char *argv[])
{

    int arr[] = {44,3,38,5,47,15,36,26,27,2,46,4,19,50,48};
    int len = sizeof(arr)/sizeof(int);

    shellsort(arr, len);
    for(int i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
