#include <rtthread.h>

class BaseClass {
  int i, j;
public:
  void set(int a, int b) { 
     i = a; 
     j = b; 
  }
  void show() { 
     rt_kprintf("i = %d, j = %d\n", i, j);
  }
};

class DerivedClass : public BaseClass {
  int k;
public:
  DerivedClass(int x) { 
     k = x; 
  }
  void showk() { 
     rt_kprintf("k = %d\n", k);
  }
};

extern "C"
{
	int cplusplus();
}
int cplusplus()
{
  DerivedClass ob(3);
  DerivedClass *ptr;

  ob.set(1, 2); // access member of BaseClass
  ob.show();    // access member of BaseClass
  ob.showk();   // uses member of DerivedClass class

  ptr = new DerivedClass(10);
  ptr->set(5, 10);
  ptr->show();
  ptr->showk();
  delete ptr;

  return 0;
}

