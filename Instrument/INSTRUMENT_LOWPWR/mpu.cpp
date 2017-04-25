#include "mpu.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

#define FSR 2000
#define ACC_SENS        16384.f          //  2.0/32768.0;
//#define GYRO_SENS       ( 131.0f * 250.f / (float)FSR )
#define GYRO_SENS       16.384f
#define QUAT_SENS       1073741824.f //2^30

#define EPSILON         0.0001f
#define PI_2            1.57079632679489661923f

struct s_mympu mympu;

struct s_quat {
  float w, x, y, z;
};

union u_quat {
  struct s_quat _f;
  long _l[4];
} q;

static int ret;
static short gyro[3];
static short accel[3];
static short sensors;
static unsigned char fifoCount;




int mympu_open(unsigned int rate) {
  mpu_select_device(0);
  mpu_init_structures();

  ret = mpu_init(NULL);
#ifdef MPU_DEBUG
  if (ret) return 10 + ret;
#endif

  ret = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
#ifdef MPU_DEBUG
  if (ret) return 20 + ret;
#endif

  ret = mpu_set_gyro_fsr(FSR);
#ifdef MPU_DEBUG
  if (ret) return 30 + ret;
#endif

  ret = mpu_set_accel_fsr(2);
#ifdef MPU_DEBUG
  if (ret) return 40 + ret;
#endif

  mpu_get_power_state((unsigned char *)&ret);
#ifdef MPU_DEBUG
  if (!ret) return 50 + ret;
#endif

  ret = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
#ifdef MPU_DEBUG
  if (ret) return 60 + ret;
#endif

  dmp_select_device(0);
  dmp_init_structures();

  ret = dmp_load_motion_driver_firmware();
#ifdef MPU_DEBUG
  if (ret) return 80 + ret;
#endif

  ret = dmp_set_fifo_rate(rate);
#ifdef MPU_DEBUG
  if (ret) return 90 + ret;
#endif

  ret = mpu_set_dmp_state(1);
#ifdef MPU_DEBUG
  if (ret) return 100 + ret;
#endif


ret = dmp_set_interrupt_mode(DMP_INT_CONTINUOUS);

mpu_set_sample_rate(20);

#ifdef MPU_DEBUG
  if (ret) return 110 + ret;
#endif

  ret = dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL | DMP_FEATURE_SEND_RAW_ACCEL);
  //	ret = dmp_enable_feature(DMP_FEATURE_SEND_CAL_GYRO|DMP_FEATURE_GYRO_CAL);
#ifdef MPU_DEBUG
  if (ret) return 110 + ret;
#endif

ret = dmp_set_fifo_rate(20);

  return 0;
}



static inline float rad2deg( float rad )
{
  //return (180.f/PI) * rad;
  return 57.2957795131f * rad;
}

static float test, sqy, sqz, sqw;

static void quaternionToEuler( const struct s_quat *q, float* x, float* y, float* z )
{
  sqy = q->y * q->y;
  sqz = q->z * q->z;
  sqw = q->w * q->w;

  test = q->x * q->z - q->w * q->y;

  if ( test > 0.5f - EPSILON )
  {
    *x = 2.f * atan2( q->y, q->w );
    *y = PI_2;
    *z = 0;
  }
  else if ( test < -0.5f + EPSILON )
  {
    *x = -2.f * atan2( q->y, q->w );
    *y = -PI_2;
    *z = 0;
  }
  else
  {
    *x = atan2( 2.f * ( q->x * q->w + q->y * q->z ), 1.f - 2.f * ( sqz + sqw ) );
    *y = asin( 2.f * test );
    *z = atan2( 2.f * ( q->x * q->y - q->z * q->w ), 1.f - 2.f * ( sqy + sqz ) );
  }
}

static inline float wrap_180(float x) {
  return (x < -180.f ? x + 360.f : (x > 180.f ? x - 360.f : x));
}

static inline float flip_180(float x) {
  return (x < 0.f ? x + 180.f : x - 180.f);
}


int mympu_read() {
  int count=0;
 
  do {
    ret = dmp_read_fifo(gyro, accel, q._l, NULL, &sensors, &fifoCount);
    /* will return:
     0 - if ok
      1 - no packet available
      2 - if BIT_FIFO_OVERFLOWN is set
      3 - if frame corrupted
           <0 - if error
    */
    count++;
    if (ret != 0) return ret;
  } while (fifoCount > 1);
  //Serial.println(count);
  count=0;


  
  q._f.w = (float)q._l[0] / (float)QUAT_SENS;
  q._f.x = (float)q._l[1] / (float)QUAT_SENS;
  q._f.y = (float)q._l[2] / (float)QUAT_SENS;
  q._f.z = (float)q._l[3] / (float)QUAT_SENS;


  quaternionToEuler( &q._f, &mympu.RL, &mympu.PT, &mympu.YW );

  /* need to adjust signs and do the wraps depending on the MPU mount orientation */
  /* if axis is no centered around 0 but around i.e 90 degree due to mount orientation */
  /* then do:  mympu.ypr[x] = wrap_180(90.f+rad2deg(mympu.ypr[x])); */
  mympu.YW = wrap_180(rad2deg(mympu.YW));
  mympu.PT = rad2deg(mympu.PT);
  mympu.RL = rad2deg(mympu.RL);


//gyro should be auto cal'd - hard code biases: -0.5725190639    -0.0305343508    -0.2290076255
  /* need to adjust signs depending on the MPU mount orientation */
  mympu.GX = (float)gyro[2] / GYRO_SENS;
  mympu.GY = (float)gyro[1] / GYRO_SENS;
  mympu.GZ = (float)gyro[0] / GYRO_SENS;

  //Hard coded biases:    -0.1484375000     0.1301879882           0.0043945312
  //0.0176391601   0.0339355468  0.0316162109
  /* need to adjust signs depending on the MPU mount orientation */

  mympu.AX = (float)accel[0] / ACC_SENS - 0.0261840820;
  mympu.AY = (float)accel[1] / ACC_SENS - 0.0240478515;
  mympu.AZ = (float)accel[2] / ACC_SENS - 0.0274658203;

  mympu.QW = q._f.w;  
  mympu.QX = q._f.x;
  mympu.QY = q._f.y;
  mympu.QZ = q._f.z;



  return 0;
}


