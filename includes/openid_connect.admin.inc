<?php

/**
 * @file
 * Administrative UI and functions for the OpenID Connect module.
 */

/**
 * Form builder: Main administrative form.
 */
function openid_connect_admin_form($form, &$form_state) {
  $client_plugins = openid_connect_get_plugins();
  $options = array();
  foreach ($client_plugins as $client_plugin) {
    $options[$client_plugin['name']] = $client_plugin['title'];
  }

  $form['#tree'] = TRUE;
  $form['openid_connect_clients_enabled'] = array(
    '#title' => t('Enable PixelPin OpenID Connect'),
    '#type' => 'checkboxes',
    '#options' => array('pixelpin' => t('Enable')),
    '#default_value' => variable_get('openid_connect_clients_enabled', array()),
  );
  foreach ($client_plugins as $client_plugin) {
    $client = openid_connect_get_client($client_plugin['name']);

    $element = 'openid_connect_clients_enabled[' . $client_plugin['name'] . ']';
    $form['clients'][$client_plugin['name']] = array(
      '#title' => $client_plugin['title'],
      '#type' => 'fieldset',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $element . '"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['clients'][$client_plugin['name']] += $client->settingsForm();
  }

  $form['openid_connect_always_save_userinfo'] = array(
    '#title' => t('Save user claims on every login'),
    '#description' => t('If disabled, user claims will only be saved when the account is first created.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('openid_connect_always_save_userinfo', TRUE),
  );

  if (variable_get('user_pictures')) {
    $form['openid_connect_user_pictures'] = array(
      '#title' => t('Fetch user profile picture from login provider'),
      '#description' => t('Whether the user profile picture from the login provider should be fetched and saved locally.'),
      '#type' => 'checkbox',
      '#default_value' => variable_get('openid_connect_user_pictures', TRUE),
    );
  }

  $form['userinfo_mapping'] = array(
    '#title' => t('User claims mapping'),
    '#type' => 'fieldset',
  );

  $user_entity_wrapper = entity_metadata_wrapper('user');
  $claims = openid_connect_claims_options();
  $properties = $user_entity_wrapper->getPropertyInfo();
  $properties_skip = _openid_connect_user_properties_to_skip();
  foreach ($properties as $property_name => $property) {
    if (isset($properties_skip[$property_name])) {
      continue;
    }
    // Always map the timezone.
    $default_value = 0;
    if ($property_name == 'timezone') {
      $default_value = 'zoneinfo';
    }
    if ($property_name == 'field_family_name_pixelpin') {
        $default_value = 'family_name';
      }
      if ($property_name == 'field_given_name_pixelpin') {
        $default_value = 'given_name';
      }
      if ($property_name == 'field_nickname_pixelpin') {
        $default_value = 'nickname';
      }
      if ($property_name == 'field_preferred_username_pixelpin') {
        $default_value = 'preferred_username';
      }
      if ($property_name == 'field_gender_pixelpin') {
        $default_value = 'gender';
      }
      if ($property_name == 'field_birthdate_pixelpin') {
        $default_value = 'birthdate';
      }
      if ($property_name == 'field_locale_pixelpin') {
        $default_value = 'locale';
      }
      if ($property_name == 'field_phone_number_pixelpin') {
        $default_value = 'phone_number';
      }
      if ($property_name == 'field_street_address_pixelpin') {
        $default_value = 'street_address';
      }
      if ($property_name == 'field_town_city_pixelpin') {
        $default_value = 'town_city';
      }
      if ($property_name == 'field_region_pixelpin') {
        $default_value = 'region';
      }
      if ($property_name == 'field_postal_code_pixelpin') {
        $default_value = 'postal_code';
      }
      if ($property_name == 'field_country_pixelpin') {
        $default_value = 'country';
      }

    $form['userinfo_mapping']['openid_connect_userinfo_mapping_property_' . $property_name] = array(
      '#type' => 'select',
      '#title' => $property['label'],
      '#description' => $property['description'],
      '#options' => $claims,
      '#empty_value' => 0,
      '#empty_option' => t('- No mapping -'),
      '#default_value' => variable_get('openid_connect_userinfo_mapping_property_' . $property_name, $default_value),
    );
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Validation handler for openid_connect_admin_form.
 */
function openid_connect_admin_form_validate($form, &$form_state) {
  $enabled_clients = $form_state['values']['openid_connect_clients_enabled'];
  $enabled_clients = array_filter($enabled_clients);
  foreach ($enabled_clients as $client_name) {
    // Give the client a chance to validate the submitted values.
    $client = openid_connect_get_client($client_name);
    $fake_form = $form['clients'][$client_name];
    $fake_form_state = array('values' => $form_state['values']['clients'][$client_name]);
    $error_element_base = 'clients][' . $client_name . '][';
    $client->settingsFormValidate($fake_form, $fake_form_state, $error_element_base);
  }
}

/**
 * Submit handler for openid_connect_admin_form.
 */
function openid_connect_admin_form_submit($form, &$form_state) {
  $enabled_clients = $form_state['values']['openid_connect_clients_enabled'];
  $enabled_clients = array_filter($enabled_clients);
  variable_set('openid_connect_clients_enabled', $enabled_clients);

  drupal_set_message(t("Configuration saved."), 'status');

  foreach ($enabled_clients as $client_name) {
    // Give the client a chance to modify the submitted values.
    $client = openid_connect_get_client($client_name);
    $fake_form = $form['clients'][$client_name];
    $fake_form_state = array('values' => $form_state['values']['clients'][$client_name]);
    $client->settingsFormSubmit($fake_form, $fake_form_state);

    // Save the settings.
    variable_set('openid_connect_client_' . $client_name, $fake_form_state['values']);
  }

  $claims = array();
  foreach ($form_state['values']['userinfo_mapping'] as $variable => $claim) {
    if (!empty($claim)) {
      $claims[] = $claim;
      variable_set($variable, $claim);
    }
    else {
      variable_del($variable);
    }
  }
  if (!empty($claims)) {
    variable_set('openid_connect_userinfo_mapping_claims', $claims);
  }
  else {
    variable_del('openid_connect_userinfo_mapping_claims');
  }

  $default_enabled = array(
    'openid_connect_always_save_userinfo',
    'openid_connect_user_pictures',
  );
  foreach ($default_enabled as $variable) {
    if (isset($form_state['values'][$variable]) && empty($form_state['values'][$variable])) {
      variable_set($variable, $form_state['values'][$variable]);
    }
  }
}
