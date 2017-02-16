<?php

/**
 * @file
 * OpenID Connect client for PixelPin.
 */

/**
 * Implements OpenID Connect Client plugin for PixelPin.
 */
class OpenIDConnectClientPixelPin extends OpenIDConnectClientBase {

  /**
   * {@inheritdoc}
   */
  public function getEndpoints() {
    return array(
      'authorization' => 'https://login.pixelpin.io/connect/authorize',
      'token' => 'https://login.pixelpin.io/connect/token',
      'userinfo' => 'https://login.pixelpin.io/connect/userinfo',
    );
  }

  /**
   * {@inheritdoc}
   */
  public function retrieveUserInfo($access_token) {
    $userinfo = parent::retrieveUserInfo($access_token);
    return $userinfo;
  }
}
