clear;
close all;

a = 9.0047e-5;
b = -0.01814533;
c = 4.3745978;

a2 = -0.018744142;
b2= 4.4376757;

x = [-40:0.01:30];
y = zeros(1,size(x,2));
y2 = zeros(1,size(x,2));
for i = 1:size(x,2)
    y(i) = a * x(i) * x(i) + b * x(i) + c;
    y2(i) = a2 * x(i) + b2;
end

figure(1);

plot(x,y,'-');

grid on;
hold on;
plot(x,y2,'-');
plot(-11.76,4.60206,'+');
plot(-20,4.77815125 ,'+');