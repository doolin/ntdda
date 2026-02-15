
#ifdef STANDALONE
int
main(int argc, char *argv[])
{
  TimeHistory * th;

   if (argc != 2) {
      printf("Usage: eqlex <motion file>\n");
      return 0;
   }

   th = getTimeHistory(argv[1], matlab);

   th_delete(th);

   return 0;
}
#endif
