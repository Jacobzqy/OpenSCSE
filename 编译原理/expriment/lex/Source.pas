begin
  integer k;
  integer function F(n);
    begin
      integer m;
      if n<=0 then F:=1
      else F:=n*F(n-1)
    end;
  read(m);
  k:=F(m);
  write(k)
end