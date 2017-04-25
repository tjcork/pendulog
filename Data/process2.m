%data plot
datafile='data.csv';

[D, T, R] = xlsread(datafile);
rTM = flip(D(:,2));                 % Data Channel
rPT = flip(D(:,4));
plot(rTM,rPT)

rTstart=start1.Position(1)
rTend=end1.Position(1)



Ts=50E-3;
Fs = 1/Ts; % sampling frequency


rsTM=rTM(find(rTM==rTstart):find(rTM==rTend));
rsPT=detrend(rPT(find(rTM==rTstart):find(rTM==rTend)));
[rsPTr,rsTMr] = resample(rsPT,rsTM,0.02,'spline');

%-----------------Q-----------------------

[rpks,rlocs]=findpeaks(rsPTr,rsTMr);


rsPTs=rsPTr(find(rsTMr==rlocs(find(rpks <= 4, 1, 'first'))):end);
rsTMs=rsTMr(find(rsTMr==rlocs(find(rpks <= 4, 1, 'first'))):end);


Q=(find(rpks <= (rsPTs(1) / exp(1)),1,'first') - find(rpks <= 4, 1, 'first'))*2*pi()



figure(2);
plot(t1TMs-t1TMs(1),t1PTs)
hold on
plot(rsTMs-rsTMs(1),rsPTs)


plot(rsTM-rsTM(1),rsPT)
ylabel('Amplitude (degrees)')
xlabel('Time (seconds)')

xticks(1000*linspace(0,100,50))
xticklabels((0:2:100))



xticks(60*1000*linspace(0,100,20))
xticklabels((0:5:100))

xlabel('Elapsed Time (hours)')
xticks(2.4e6*[0 2 4 6 8 10 12 14 16 18 20 22])
xticklabels({'0','2','4','6','8','10','12','14','16','18','20','22'})



%-------------FFT2------------------------


data=rsPT;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,x]=max(abs(freqamp(100:end)));
f(x+99)
figure
hold on
plot(f,2*abs(freqamp))



data=t1PTs;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,x]=max(abs(freqamp(100:end)));
f(x+99)
%figure(3)
hold on
plot(f,2*abs(freqamp))







