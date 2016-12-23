close all;
clear;

load('AD_data_1.mat');
[a,b] = polyfit(AD_data_1(:,1),AD_data_1(:,2),1);
y = polyval(a,AD_data_1(:,1));

figure(1);
hold on;
grid on;
plot(AD_data_1(:,1),AD_data_1(:,2),'+');
plot(AD_data_1(:,1),y,'-');

title(['y = ',num2str(a(1)),'x + ',num2str(a(2))]);

legend('实际值','拟合直线');

corrcoef(AD_data_1(:,1),AD_data_1(:,2))