<?php
use PHPUnit\Framework\TestCase;

class DrushTests extends TestCase {
  public function testDrushSt() {
    exec('drush st', $output);
    $this->assertStringContainsString('Backdrop version', $output[0]);
    $this->assertStringContainsString('Backdrop bootstrap', $output[7]);
    $this->assertStringContainsString('Successful', $output[7]);
    $this->assertStringContainsString('PHP OS',$output[10]);
    $this->assertStringContainsString('Backdrop Settings File', $output[18]);
  }

  public function testPmDownload() {
    exec("drush dl devel -n", $output);
    $this->assertStringContainsString('Success:', $output[1]);
    $this->assertStringContainsString('devel', $output[1]);
    $this->assertDirectoryExists('/app/web/modules/devel');
    // clean up.
    exec('rm -rf /app/web/modules/devel');
  }
}
