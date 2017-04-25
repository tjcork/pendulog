[D, T, R] = xlsread('data.csv');
TM = D(:,2);                 % Data Channel
PT = D(:,4);




%Ts = (TM(1)-TM(end))*1E-3/(length(TM)-1)
Ts=50E-3;
Fs = 1/Ts; % sampling frequency
[y, Ty] = resample(PT,TM,0.02);
%plot(TM,PT,'.-', Ty,y,'o-')

data=resample(PT,TM,10);

%plot(linspace(TM(end),TM(1),length(TM)),data)
plot(linspace(0,TM(1)-TM(end),length(data)),data)
ylabel('Pitch (degrees)')
xlabel('Elapsed Time (hours)')
xticks(3.6e6*[0 2 4 6 8 10 12 14 16 18 20 22])
xticklabels({'0','2','4','6','8','10','12','14','16','18','20','22'})


n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,i]=max(freqamp(freqamp<max(freqamp)));

f(i)

% Plot single-sided amplitude spectrum.
figure(2);
plot(f,2*abs(freqamp)) 
title('Single-Sided Amplitude Spectrum of y(t)')
xlabel('Frequency (Hz)')
ylabel('|Y(f)|')


TM=wTM(100000:1000000);
PT=wPT(100000:1000000);

Ts=50E-3;
Fs = 1/Ts; % sampling frequency

[y, Ty] = resample(PT,TM,0.02);

data=y;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,i]=max(abs(freqamp(1000:end)));
f(i+999)
% Plot single-sided amplitude spectrum.
figure(2);
plot(f,2*abs(freqamp))
title('Single-Sided Amplitude Spectrum of y(t)')
xlabel('Frequency (Hz)')
ylabel('|Y(f)|')




%plot(linspace(TM(end),TM(1),length(TM)),data)
plot(Ty,y)
ylabel('Pitch (degrees)')
xlabel('Elapsed Time (hours)')
xticks(3.6e6*[0 2 4 6 8 10 12 14 16 18 20 22])
xticklabels({'0','2','4','6','8','10','12','14','16','18','20','22'})

