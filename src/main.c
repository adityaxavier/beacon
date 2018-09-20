#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "host/ble_hs.h"
#include "host/util/util.h"

uint8_t own_addr_type;

void set_random_addr(void)
{
    printf("Fetching Device Address\n");
    int err = ble_hs_util_ensure_addr(0);
    if (err != 0)
    {
        printf("Error ensuring address type :- %d\n", err);
        return;
    }
    else
    {
        /* Figure out address to use while advertising (no privacy for now) */
        err = ble_hs_id_infer_auto(0, &own_addr_type);
        if (err != 0)
        {
            printf("Error infering address auto :- %d\n", err);
            return;
        }
        else
        {
            printf("Infered Address\n");
        }
    }
}

void beacon_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    uint8_t uuid128[16];
    int rc;

    /* Arbitrarily set the UUID to a string of 0x11 bytes. */
    memset(uuid128, 0x11, sizeof uuid128);

    /* Major version=2; minor version=10. */
    rc = ble_ibeacon_set_adv_data(uuid128, 2, 10);
    assert(rc == 0);

    /* Begin advertising. */
    adv_params = (struct ble_gap_adv_params){0};
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, NULL, NULL);

    assert(rc == 0);
}


static void
ble_app_on_sync(void)
{
    /* Advertise indefinitely. */
    set_random_addr();
    beacon_advertise();
}


int main(int argc, char **argv)
{
    sysinit();

    ble_hs_cfg.sync_cb = ble_app_on_sync;

    /* As the last thing, process events from default event queue. */
    while (1)
    {
        os_eventq_run(os_eventq_dflt_get());
    }
}