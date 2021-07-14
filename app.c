/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"
#include "app_gattdb.h"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;




// Handle for dynamic GATT Database session.
uint16_t gattdb_session;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}


/**************************************************************************//**
 * Initialize GATT database dynamically.
 *****************************************************************************/
static void initialize_gatt_database(void)
{
  sl_status_t sc;

  // New session
  sc = sl_bt_gattdb_new_session(&gattdb_session);
  app_assert_status(sc);

  // Add services
  for (service_index_t i = (service_index_t)0; i < SERVICES_COUNT; i++) {
    sc = app_gattdb_add_service(gattdb_session, &services[i]);
    app_assert_status(sc);
  }

  // Add characteristics
  for (characteristic_index_t i = (characteristic_index_t)0; i < CHARACTERISTICS_COUNT; i++) {
    sc = app_gattdb_add_characteristic(gattdb_session, &characteristics[i]);
    app_assert_status(sc);
  }

  // Start services and child characteristics
  for (service_index_t i = (service_index_t)0; i < SERVICES_COUNT; i++) {
    sc = sl_bt_gattdb_start_service(gattdb_session, services[i].handle);
    app_assert_status(sc);
  }

  // Commit changes
  sc = sl_bt_gattdb_commit(gattdb_session);
  app_assert_status(sc);
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;


  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      //
    //  sl_bt_gattdb_new_session  ( &GATTdb_session0 );
    //  sl_bt_gattdb_add_service(GATTdb_session0, sl_bt_gattdb_primary_service, SL_BT_GATTDB_ADVERTISED_SERVICE, sizeof(DynamicServiceUUID), &DynamicServiceUUID, &DynamicService);
    //  sl_bt_gattdb_add_uuid16_characteristic(GATTdb_session0, DynamicService, SL_BT_GATTDB_CHARACTERISTIC_READ, 0, 0, uuid, value_type, maxlen, value_len, value, characteristic)

      // Initialize GATT database dynamically.
            initialize_gatt_database();

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Restart advertising after client has disconnected.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}
