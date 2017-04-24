%data plot
datafile='data (5).csv';

[D, T, R] = xlsread(datafile);
aTM=flip(D(:,2));
AX=flip(D(:,6));
AY=flip(D(:,7));
AZ=flip(D(:,8));
figure;
hold on
[AX,nTM] = resample(AX,aTM,0.02,'spline');
plot(nTM,AX)
[AY,nTM] = resample(AY,aTM,0.02,'spline');
plot(nTM,AY)
[AZ,nTM] = resample(AZ,aTM,0.02,'spline');
plot(nTM,AZ)


astart=s1.Position(1);
aend=e1.Position(1);

anTM=nTM(find(nTM==astart):find(nTM==aend));
nAX=detrend(AX(find(nTM==astart):find(nTM==aend)));
nAY=detrend(AY(find(nTM==astart):find(nTM==aend)));
nAZ=detrend(AZ(find(nTM==astart):find(nTM==aend)));

figure
hold on;
plot(anTM,detrend(nAX))
plot(anTM,detrend(nAY))
plot(anTM,detrend(nAZ))


d = fdesign.lowpass('Fp,Fst,Ap,Ast',1.5,2,0.1,100,20);
   Hd = design(d,'equiripple');
   fAX= filter(Hd,nAX);
   fAY= filter(Hd,nAY);
   fAZ= filter(Hd,nAZ);
   
figure
hold on
plot(anTM-anTM(1),fAX)
plot(anTM-anTM(1),fAY+1000)
plot(anTM-anTM(1),fAZ)
xlabel('Time (seconds)')
xticks(1000*linspace(0,100,25))
xticklabels({'0','0','4','8','12','16','20','24','28','32','36'})





data=nAZ;
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





rTstart=rStart.Position(1)
rTend=rEnd.Position(1)



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



ylabel('Amplitude (degrees)')
xlabel('Time (minutes)')
xticks(60*1000*[0 2 4 6 8 10 12 14 16 18 20 22])
xticklabels({'0','2','4','6','8','10','12','14','16','18','20','22'})

%-------------FFT2------------------------


data=rsPTs;
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







