syms Vm R U;
a = 9.0047e-5;
b = -0.01814533;
c = 4.3745978;
%log((Vm/U-1)*R) = ax^2+bx+c
Vm_regular = 1.2;
R_regular = 10000;

x = (-b - sqrt(b^2-4*a*(c-log10((Vm/U-1) * R))))/(2*a);
ddVm = diff(x,Vm);
ddVm_VmR_value = subs(ddVm,[Vm,R],[Vm_regular,R_regular]);
ddR_VmR_value = subs(diff(x,R),[Vm,R],[Vm_regular,R_regular]);
%vpa(Uf,5)
for i = 1:6
    ddVm_value = vpa(subs(ddVm_VmR_value,U,i*0.1),5);
    fprintf('U=%f temp=%f ddVm=%f %%5 dif=%f\n',i*0.1...
        ,vpa(subs(x,[Vm R U],[Vm_regular,R_regular,i*0.1]),5)...
        ,ddVm_value,Vm_regular*0.05*ddVm_value);
    ddR_value = vpa(subs(ddR_VmR_value,U,i*0.1),5);
    fprintf('U=%f temp=%f ddR=%f %%5 dif=%f\n',i*0.1...
        ,vpa(subs(x,[Vm R U],[Vm_regular,R_regular,i*0.1]),5)...
        ,ddR_value,R_regular*0.05*ddR_value);
end