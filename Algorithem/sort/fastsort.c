/*
 * 快速排序
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void swap(int *x, int *y)
{
    int t = *x;
    *x = *y;
    *y = t;
}

#if 0
void fastsort(int *arr, int len)
{
    if(len < 2)
        return;

    int left = 0;
    int right = len - 1;
    int temp = arr[0];
    int select_value = 0;
    int continue_flag = 0;

    while(left < right)
    {
        if(continue_flag == 0)
        {
            if(arr[right] > temp)
            {
                right--;
                continue;
            }
            else 
            {
                arr[left] = arr[right];
                continue_flag = 1;
            }
        }
        else
        {
            if(arr[left] <= temp)
            {
                left++;
                continue;
            }
            else
            {
                arr[right] = arr[left];
                continue_flag = 0;
            }
        }
    }
    arr[right] = temp;
    fastsort(arr, left);
    fastsort(arr+left+1, len - left - 1);
}
#else

void _fastsort(int *arr, int start, int end)
{
    if(start >= end)
        return;

    int temp = arr[start];
    int left = start+1;
    int right = end;

    while(left < right)
    {
        while(arr[left] < temp && left < right)
            left++;
        while(arr[right] >= temp && left < right)
            right--;
        swap(arr+left, arr+right);
    }
    for(int i = start; i < end; i++)
        printf("%d ", arr[i]);
    printf("\n");
    printf("mid value = %d\n", arr[left]);
    printf("\n");
    usleep(10000);
    if(arr[left] < temp)
        swap(arr+start, arr+left);
    _fastsort(arr, start, left -1);
    _fastsort(arr, right, end);
    
}

void fastsort(int *arr, int len)
{
    _fastsort(arr, 0, len - 1);    
}

#endif

int main(int argc, char *argv[])
{
    int arr[] = {44,3,3,38,5,47,15, 36,26,27,2,46,4,19,50,48};
    int len = sizeof(arr)/sizeof(int);

    fastsort(arr, len);

    for(int i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
