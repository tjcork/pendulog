
<?php
include '../class.dB.php';

// Our database object
$db = new Db();   
?>

<html class="no-js" lang="">
    <head>
        <meta charset="utf-8">
        <meta http-equiv="x-ua-compatible" content="ie=edge">
        <title>Pendulog</title>
        <meta name="description" content="">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="icon" type="image/png" sizes="180x180" href="apple-touch-icon.png">
        <link rel="icon" type="image/png" href="favicon-32x32.png" sizes="32x32">
        <link rel="icon" type="image/png" href="favicon-16x16.png" sizes="16x16">
        <link rel="manifest" href="manifest.json">
        <link rel="mask-icon" href="safari-pinned-tab.svg">
        <meta name="theme-color" content="#ffffff">
        <!-- Place favicon.ico in the root directory -->

        <link rel="stylesheet" href="css/normalize.css">
        <link rel="stylesheet" href="css/main.css">
        <link rel="stylesheet" href="css/styles.css">
        <script src="js/vendor/modernizr-2.8.3.min.js"></script>
        
        <script src="./src/Chart.bundle.js"></script>
       <!-- <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/1.0.2/Chart.min.js"></script>-->
        <script src="./src/utils.js"></script>
        
    </head>
    <body onload="startPendulum()">
        <!--[if lt IE 8]>
            <p class="browserupgrade">You are using an <strong>outdated</strong> browser. Please <a href="http://browsehappy.com/">upgrade your browser</a> to improve your experience.</p>
        <![endif]-->

        <!-- Add your site or application content here -->
    <!--    <div class="root">
        <div id="pendulum" class="wb_element"><img alt="pendulum" src="pendulum.png"></div>
    --> <div id="title" class="wb_element" style=" line-height: normal;"><h5 class="wb-stl-subtitle">Pendulog</h5></div>
        <div id="subtitle" class="wb_element" style=" line-height: normal;"><p class="wb-stl-normal"> An IoT pendulum motion logger</p></div>
       
           
        
    <canvas id="myCanvas" width="992" height="700"></canvas>
    <script>
          
      window.requestAnimFrame = (function(callback) {
        return window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.oRequestAnimationFrame || window.msRequestAnimationFrame ||
        function(callback) {
          window.setTimeout(callback, 1000 / 20);
        };
      })();

      function drawPendulum(myPendulum, context) {
        context.save();
        context.translate(canvas.width/2, -500);
        context.rotate(myPendulum.angle*Math.PI/180);
        context.drawImage(myPendulum,-myPendulum.width/2, 480);
        context.restore();
        
      }
      
      
      function animate(myPendulum, canvas, context, startTime) {
        // update
        var time = (new Date()).getTime() - startTime;
        var amplitude = 150;
        var angamplitude=8;

        // in ms
        var period = 2000;
        var nextAng= angamplitude * Math.sin(time * 2 * Math.PI / period);
        myPendulum.angle = nextAng;

        // clear
        context.clearRect(0, 0, canvas.width, canvas.height);

        // draw
        
        drawPendulum(myPendulum, context);

        // request new frame
        requestAnimFrame(function() {
          animate(myPendulum, canvas, context, startTime);
        });
      }
      
      var canvas = document.getElementById('myCanvas');
      var context = canvas.getContext('2d');
      var myPendulum = new Image();
      myPendulum.angle=0;
      
      myPendulum.onload = function() {
        context.drawImage(myPendulum, canvas.width/2, 0);
        
      };
      myPendulum.src = 'pendulum.png';
      
      
      var startTime = (new Date()).getTime();
      animate(myPendulum, canvas, context, startTime);
      
          
      
      
      </script>
      



      
<?php 

$YawRet=$db -> query("SELECT Time,Yaw,Pitch,Roll FROM data ORDER BY id LIMIT 100");
$YawData = array();
$PitchData = array();
$RollData = array();
$TimeData = array();
while($row = mysqli_fetch_assoc($YawRet)) {
   $YawData[] = $row['Yaw'];
   $PitchData[] = $row['Pitch'];
   $RollData[] = $row['Roll'];
   $TimeData[] = $row['Time'];
   
}

?>      
      
      
      
    <div class='plots'; style="width:100%; margin:auto;">
        <canvas id="plot"></canvas>
    </div>

    <script>
        var idRef=101;
        var MONTHS = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
        Chart.defaults.global.tooltips.enabled = false;       
        var config = {
            type: 'line',
            animationEnabled: false,
            data: {
                labels: <?php echo json_encode($TimeData);?>,    
                datasets: [{
                    label: "Yaw",
                    backgroundColor: window.chartColors.red,
                    borderColor: window.chartColors.red,
                    data: <?php echo json_encode($YawData);?>,
                    fill: false,
                    pointRadius:0
                }, {
                    label: "Pitch",
                    backgroundColor: window.chartColors.blue,
                    borderColor: window.chartColors.blue,
                    data: <?php echo json_encode($PitchData);?>,
                    fill: false,
                    pointRadius: 0
                }, {
                    label: "Roll",
                    backgroundColor: window.chartColors.orange,
                    borderColor: window.chartColors.orange,
                    data: <?php echo json_encode($RollData);?>,
                    fill: false,
                    pointRadius: 0
                }]       
            },
            options: {
                responsive: true,
                title:{
                    display:false,
                    text:'Stream'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                hover: {
                    mode: 'nearest',
                    intersect: true
                },
                scales: {
                    xAxes: [{
                       type: 'time',
                       time: {
                            displayFormats: {
                                millisecond: 'SSS [ms]'
                            }
                        },
                        display: false,
                        scaleLabel: {
                            display: true,
                            labelString: 'Month'
                        }
                    }],
                    yAxes: [{
                        display: true,
                        scaleLabel: {
                            display: true,
                            labelString: 'Degrees'
                        },
                        ticks: {
                            suggestedMin: -180,    // minimum will be 0, unless there is a lower value.
                            max: 180,
                            min: -180,
                            stepSize: 60
                        }
                    }]
                }
            }
        };

        window.onload = function() {
            var ctx = document.getElementById("plot").getContext("2d");
            window.myLine = new Chart(ctx, config);
        };
        
        
        
        function removeLastData() {
            config.data.labels.splice(-1, 1); // remove the label first
            config.data.datasets.forEach(function(dataset) {
                dataset.data.pop();
            });
            window.myLine.update();       
        }
        
        function removeFirstData() {
           config.data.datasets.forEach(function(dataset) {
                dataset.data.splice(0, 1);

            });
            config.data.labels.splice(0, 1); // remove the label first
            window.myLine.update();
 
            
        }
        
        function addData() {
           $.ajax({                                      
              url: '../../api.php',                  //the script to call to get data          
              data: "id="+idRef,                        //you can insert url argumnets here to pass to api.php
              type: "GET",                         
              dataType: 'json',                //data format      
              success: function f(data)          //on recieve of reply
              {
                if (jQuery.isEmptyObject(data)){
                    //pause until new data
                    return false;
                }
                config.data.labels.push(data[0].Time);  
                config.data.datasets[0].data.push(data[0].Yaw);
                config.data.datasets[1].data.push(data[0].Pitch);
                config.data.datasets[2].data.push(data[0].Roll);
                
                window.myLine.update();
                //remove first set
                config.data.datasets.forEach(function(dataset) {
                    dataset.data.splice(0, 1);

                });
                config.data.labels.splice(0, 1); // remove the label first
                window.myLine.update();
                idRef=idRef+1;
              } 
            });
            return;
        }
        
        function scrollData() {
            addData();
         }
        
        window.setInterval(scrollData, 250);        
                
    </script>

   <!-- <br><br><div class="test"></div>-->
    <script>
/*
      			var futureDate = new Date();
			
			function getStringFormat(d){
				function getFullDay(d){
					var weekday = ["Sunday", "Monday","Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
					return weekday[d.getDay()];
				}

				function getFullMonth(d){
					var months = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
					return months[d.getMonth()]
				}
				return getFullDay(d) +", " + getFullMonth(d) +" "+ d.getDate() + ", " + d.getFullYear() + " " +  d.getHours()+":"+ d.getMinutes() +":" + d.getSeconds()
			}
			
			//first deadline
			function getNewFutureTime(t){
				futureDate.setTime(futureDate.getTime() + t * 1000);
				return getStringFormat(futureDate)
				// oned
			}
			$(".test").html(getNewFutureTime(60));
			setInterval(function(){
				$(".test").html(getNewFutureTime(30));
			}, 2 * 1000)
                        
                        
   */                     
             </script>
      
      
      
      
      
      
      
      
        <script src="https://code.jquery.com/jquery-1.12.0.min.js"></script>
        <script>window.jQuery || document.write('<script src="js/vendor/jquery-1.12.0.min.js"><\/script>')</script>
        <script src="js/plugins.js"></script>
        <script src="js/main.js"></script>

        <!-- Google Analytics: change UA-XXXXX-X to be your site's ID. -->
        <script>
            (function(b,o,i,l,e,r){b.GoogleAnalyticsObject=l;b[l]||(b[l]=
            function(){(b[l].q=b[l].q||[]).push(arguments)});b[l].l=+new Date;
            e=o.createElement(i);r=o.getElementsByTagName(i)[0];
            e.src='https://www.google-analytics.com/analytics.js';
            r.parentNode.insertBefore(e,r)}(window,document,'script','ga'));
            ga('create','UA-88075869-1','auto');ga('send','pageview');
        </script>
    </body>
</html>
