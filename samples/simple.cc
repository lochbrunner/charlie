// Simple test

int g = 4;

int cubic(int v)
{
  return v *v*v;
}

int main()
{
  int i = 7;

  println(2 * g + i);

  println(cubic(4 - 1) - 12);

  if (1) 
  {
  	println(123);
  }
  g++;
  println(g);
  g--;
  println(g);
}