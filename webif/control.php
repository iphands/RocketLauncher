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
$out = shell_exec('pgrep rocketLauncher >/dev/null 2>&1 ; echo $?');
if ($out != 0) {
  $out = shell_exec('/usr/local/sbin/rocketLauncher ' . $direction . ' ' . $time);
  $out = "<br>\n" . preg_replace('/\n/', '<br>', $out);
  print($out);
}
?>
