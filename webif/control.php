<?php
$direction = $_REQUEST['direction'];
$time = $_REQUEST['time'];

$direction = preg_split("/[;&|!@#$%^&*\(\)]/", $direction);
$direction = $direction[0];

$time = preg_split("/[;&|!@#$%^&*\(\)]/", $time);
$time = $time[0];

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
