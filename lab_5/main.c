#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dplist.h"

typedef struct _Employee {
    char name[32];
    unsigned char age;
}Employee;

Employee *copy_employee(Employee *employee) {
    Employee *new_employee = (Employee*)malloc(sizeof(Employee));
    strcpy(new_employee->name, employee->name);
    new_employee->age = employee->age;
    return new_employee;
}

void **free_employee(Employee **employee) {
    free(*employee);
    *employee = NULL;
    return NULL;
}

int compare_employee(Employee *employee1, Employee *employee2) {
    if(employee1->age > employee2->age) return 1;
    if(employee1->age < employee2->age) return -1;
    return 0;
}


int main() {
    
    dplist_t * list;
    
    Employee *samuel = (Employee*) malloc(sizeof(Employee));
    strcpy(samuel->name, "Samuel");
    samuel->age = 32;
    
    Employee *sally = (Employee*) malloc(sizeof(Employee));
    strcpy(sally->name, "Sally");
    sally->age = 28;

    Employee *susan = (Employee*) malloc(sizeof(Employee));
    strcpy(susan->name, "Susan");
    susan->age = 45;

    Employee *marc =(Employee*) malloc(sizeof(Employee));
    strcpy(marc->name, "Marc");
    marc->age=35;

    list = dpl_create(  (void *(*)(void *))copy_employee,
                        (void (*)(void **))free_employee,
                        (int (*)(void *, void *))compare_employee
                    );
    
    list=dpl_insert_at_index(list, samuel, 0, false);
    list=dpl_insert_at_index(list, sally,1,false);
    list=dpl_insert_at_index(list,susan,2,false);

    //test 1
    assert(dpl_get_element_at_reference(list,dpl_get_first_reference(list))==samuel);
    assert(dpl_get_element_at_reference(list,dpl_get_last_reference(list))==susan);
    assert(dpl_get_element_at_reference(list,dpl_get_next_reference(list,dpl_get_first_reference(list)))==sally);
    assert(dpl_get_element_at_reference(list,dpl_get_previous_reference(list,dpl_get_last_reference(list)))==sally);
      
    printf("Test 1 was succesful\n");

    //test 2
    list=dpl_insert_at_reference(list,marc,dpl_get_reference_at_index(list,1),false);
    assert(dpl_get_element_at_index(list,1)==marc);

    printf("Test 2 was succesful\n");

    dpl_remove_at_index(list,0,false);
    dpl_remove_at_index(list,0,false);
    dpl_remove_at_index(list,0,false);
    dpl_remove_at_index(list,0,false);
    assert(dpl_size(list)==0);
    //test3
    
    list=dpl_insert_sorted(list,samuel,false);
    list=dpl_insert_sorted(list,susan,false);
    list=dpl_insert_sorted(list,sally,false);
    list=dpl_insert_sorted(list,marc,false);
    assert(dpl_size(list)==4);

    assert(dpl_get_element_at_index(list,0)==sally);
    assert(dpl_get_element_at_index(list,1)==samuel);
    assert(dpl_get_element_at_index(list,2)==marc);
    assert(dpl_get_element_at_index(list,3)==susan);
    printf("Test 3 was succesful\n");

    list=dpl_remove_at_reference(list,dpl_get_reference_at_index(list,2),false);
    assert(dpl_get_element_at_index(list,2)==susan);
    printf("Test 4 was succesful\n");

    list=dpl_remove_element(list,samuel,false);
    assert(dpl_get_element_at_index(list,1)==susan);
    list=dpl_remove_element(list,marc,false);
    printf("Test 5 was succesful\n");

    dpl_free(&list, false);
    free(samuel); 
    free(sally);
    free(susan);
    free(marc);
    
    return 0;
}

