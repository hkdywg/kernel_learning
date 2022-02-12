/*
 * 归并排序
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void _mergesort(int *arr, int *temp, int start, int end)
{
    if(start >= end)
        return;
    int mid = start + (end - start)/2;
    int left_start = start;
    int left_end = mid;
    int right_start = mid + 1;
    int right_end = end;

    _mergesort(arr, temp, left_start, left_end);
    _mergesort(arr, temp, right_start, right_end);

    int i = start;
    while(left_start <= left_end && right_start <= right_end)
        temp[i++] = arr[left_start] < arr[right_start] ? arr[left_start++] : arr[right_start++];
#if 1
    if(left_start <= left_end)
        memcpy(temp+i, arr+left_start, (left_end-left_start+1)*sizeof(int));
    if(right_start <= right_end)
        memcpy(temp+i, arr+right_start, (right_end-right_start+1)*sizeof(int));
#else
    while(left_start <= left_end)
        temp[i++] = arr[left_start++];
    while(right_start <= right_end)
        temp[i++] = arr[right_start++];
#endif

    memcpy(arr+start, temp+start, (end-start+1)*sizeof(int));

}


void mergesort(int *arr, int len)
{
    int temparr[len];
   _mergesort(arr, temparr, 0, len-1); 
}

int main(int argc, char *argv[])
{
    int arr[] = {44,3,38,5,47,15,36,26,27,2,46,4,19,50,48};
    int len = sizeof(arr)/sizeof(int);

    mergesort(arr, len);

    for(int i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");
    
    return 0;
}
