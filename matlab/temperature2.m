close all;
clear;

load('temp_data_1.mat');
[a,b] = polyfit(A(:,1),A(:,2),1);
y = polyval(a,A(:,1));

figure(1);
hold on;
grid on;
plot(A(:,1),A(:,2),'+');
plot(A(:,1),y,'-');

title(['y = ',num2str(a(1)),'x + ',num2str(a(2))]);

legend('实际值','拟合直线');

corrcoef(A(:,1),A(:,2))

[a,b] = polyfit(B(:,1),B(:,2),1);
y = polyval(a,B(:,1));

figure(2);
hold on;
grid on;
plot(B(:,1),B(:,2),'+');
plot(B(:,1),y,'-');

title(['y = ',num2str(a(1)),'x + ',num2str(a(2))]);

legend('实际值','拟合直线');

corrcoef(B(:,1),B(:,2))