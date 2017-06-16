<?php

/**
 * @file
 * Forms for OpenID Connect.
 */

/**
 * Adds a SSO button for PixelPin for the login form page.
 * 
 * @param type $form
 * @param type $form_state
 * @return form element
 */
function openid_connect_login_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'openid_connect', 'includes/openid_connect.forms');

  foreach (openid_connect_get_plugins(TRUE) as $client) {
    $form['openid_connect_client_pixelpin_login'] = array(
      '#type' => 'submit',
      '#value' => t('Log in Using PixelPin'),
      '#name' => $client['name'],
    );
  }

  return $form;
}

/**
 * Adds a SSO button for PixelPin for the front page.
 * 
 * @param type $form
 * @param type $form_state
 * @return form element
 */
function openid_connect_login_front_page_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'openid_connect', 'includes/openid_connect.forms');

  foreach (openid_connect_get_plugins(TRUE) as $client) {
    $form['openid_connect_client_pixelpin_login_front_page'] = array(
      '#type' => 'submit',
      '#value' => t('Log in Using PixelPin'),
      '#name' => $client['name'],
    );
  }

  return $form;
}

/**
 * Adds a SSO button for PixelPin for the registration form page.
 * 
 * @param type $form
 * @param type $form_state
 * @return form element
 */
function openid_connect_register_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'openid_connect', 'includes/openid_connect.forms');

  foreach (openid_connect_get_plugins(TRUE) as $client) {
    $form['openid_connect_client_pixelpin_register'] = array(
      '#type' => 'submit',
      '#value' => t('Register Using PixelPin'),
      '#name' => $client['name'],
    );
  }

  return $form;
}

/**
 * Form submit handler: Log in with an OpenID Connect login provider.
 * 
 * @param type $form
 * @param type $form_state
 */
function openid_connect_login_form_submit(&$form, &$form_state) {
  openid_connect_save_destination();

  $client_name = $form_state['triggering_element']['#name'];
  $client = openid_connect_get_client($client_name);
  $scopes = openid_connect_get_scopes();
  $_SESSION['openid_connect_op'] = 'login';
  $client->authorize($scopes);
}

/**
 * Form submit handler: Register with an OpenID Connect login provider.
 * 
 * @param type $form
 * @param type $form_state
 */
function openid_connect_register_form_submit(&$form, &$form_state) {
  openid_connect_save_destination();

  $client_name = $form_state['triggering_element']['#name'];
  $client = openid_connect_get_client($client_name);
  $scopes = openid_connect_get_scopes();
  $_SESSION['openid_connect_op'] = 'login';
  $client->authorize($scopes);
}

/**
 * Form submit handler: Log in with an OpenID Connect login provider.
 * 
 * @param type $form
 * @param type $form_state
 */
function openid_connect_login_front_page_form_submit(&$form, &$form_state) {
  openid_connect_save_destination();

  $client_name = $form_state['triggering_element']['#name'];
  $client = openid_connect_get_client($client_name);
  $scopes = openid_connect_get_scopes();
  $_SESSION['openid_connect_op'] = 'login';
  $client->authorize($scopes);
}

/**
 * Form builder: Connect an external account to your existing Drupal account.
 * 
 * @param type $form
 * @param array $form_state
 * @param type $account
 * @return form element
 */
function openid_connect_connect_form($form, &$form_state, $account) {
  form_load_include($form_state, 'inc', 'openid_connect', 'includes/openid_connect.forms');

  $form_state['account'] = $account;

  $clients = openid_connect_get_plugins(TRUE);

  $read_only = $GLOBALS['user']->uid != $account->uid;

  $form['help'] = array(
    '#prefix' => '<p class="description">',
    '#suffix' => '</p>',
  );

  if (empty($clients)) {
    $form['help']['#markup'] = t('No external account providers are available.');

    return $form;
  }
  elseif ($GLOBALS['user']->uid == $account->uid) {
    $form['help']['#markup'] = t('You can connect your account with these external providers.');
  }

  $connected_accounts = openid_connect_get_connected_accounts($account);

  foreach ($clients as $client) {
    $form[$client['name']] = array(
      '#type' => 'fieldset',
      '#title' => t('Provider: PixelPin', array('@title' => $client['title'])),
    );
    $fieldset = &$form[$client['name']];
    $connected = isset($connected_accounts[$client['name']]);
    $fieldset['status'] = array(
      '#type' => 'item',
      '#title' => t('Status'),
      '#markup' => t('Not connected'),
    );
    if ($connected) {
      $fieldset['status']['#markup'] = t('Connected as %sub', array(
        '%sub' => $connected_accounts[$client['name']],
      ));
      $fieldset['openid_connect_client_' . $client['name'] . '_disconnect'] = array(
        '#type' => 'submit',
        '#value' => t('Disconnect from PixelPin'),
        '#name' => 'disconnect__' . $client['name'],
        '#access' => !$read_only,
      );
    }
    else {
      $fieldset['status']['#markup'] = t('Not connected');
      $fieldset['openid_connect_client_' . $client['name'] . '_connect'] = array(
        '#type' => 'submit',
        '#value' => t('Connect with PixelPin'),
        '#name' => 'connect__' . $client['name'],
        '#access' => !$read_only,
      );
    }
  }

  return $form;
}

/**
 * Submit callback for the connect account form.
 *
 * Connect the current user's Drupal account to an external provider.
 * 
 * @global type $user
 * @param type $form
 * @param type $form_state
 * @return type
 */
function openid_connect_connect_form_submit(&$form, &$form_state) {
  list($op, $client_name) = explode('__', $form_state['triggering_element']['#name'], 2);

  if ($op === 'disconnect') {
    openid_connect_disconnect_account($form_state['account'], $client_name);
    $client = openid_connect_get_client($client_name);
    drupal_set_message(t('Account successfully disconnected from PixelPin'));

    return;
  }

  global $user;

  if ($user->uid !== $form_state['account']->uid) {
    drupal_set_message(t("You cannot connect another user's account."), 'error');

    return;
  }

  openid_connect_save_destination();

  $client = openid_connect_get_client($client_name);
  $scopes = openid_connect_get_scopes();
  $_SESSION['openid_connect_op'] = $op;
  $_SESSION['openid_connect_connect_uid'] = $user->uid;
  $client->authorize($scopes);
}
