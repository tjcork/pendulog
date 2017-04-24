[fD,fT] = resample(wPT,wTM,0.02,'spline');

[pks,locs]=findpeaks(fD,fT)
[nP,nL] = resample(pks,locs,0.02,'spline');


data=nP;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);

% Plot single-sided amplitude spectrum.
figure(2);
plot(f,2*abs(freqamp))
title('Single-Sided Amplitude Spectrum of y(t)')
xlabel('Frequency (Hz)')
ylabel('|Y(f)|')


