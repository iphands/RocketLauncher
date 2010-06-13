<?php
  // url = url + "?direction=" + direction;
  // url = url +" &time=" + time;
$direction = $_REQUEST['direction'];
$time = $_REQUEST['time'];

if (($time == NULL) || ($direction == NULL)) {
  print("error with args. direction and time needed");
  return;
}

print($direction . " " . $time . "\n");
$out = shell_exec('/usr/local/sbin/rocketLauncher ' . $direction . ' ' . $time);
print($out);
?>