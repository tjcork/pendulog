[D, T, R] = xlsread('data.csv');
TM = D(:,2);                 % Data Channel
PT = D(:,4);




Ts=50E-3;
Fs = 1/Ts; % sampling frequency
interval=72000;
for i=0:interval:1000000-interval
    TM=wTM(i+1:i+interval);
    PT=wPT(i+1:i+interval);
    [data,Ty] = resample(PT,TM,0.02,'spline');
    n = length(data);
    NFFT = 2^nextpow2(n); % Next power of 2 from length of data
    Y = fft(data,NFFT)/n;
    f = Fs/2*linspace(0,1,NFFT/2+1);
    freqamp=Y(1:NFFT/2+1);
    [d,x]=max(abs(freqamp(100:end)));
    f(x+99)
    hold on 
    plot(f,2*abs(freqamp))
end

xlabel('Frequency (Hz)')
ylabel('|Y(f)|')




% Plot single-sided amplitude spectrum.
figure(2);
plot(f,2*abs(freqamp))
title('Single-Sided Amplitude Spectrum of y(t)')
xlabel('Frequency (Hz)')
ylabel('|Y(f)|')

