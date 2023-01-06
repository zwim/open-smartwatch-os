#include "./apps/clock/OswAppAlarm.h"

void OswAppAlarm::setup()
{
}

void OswAppAlarm::stop()
{
}

OswAppAlarm::OswAppAlarm(OswAppSwitcher *clockAppSwitcher)
{
    this->clockAppSwitcher = clockAppSwitcher;
    state = AlarmState::IDLE;
    notifications = notifierClient.readNotifications();
}

void OswAppAlarm::handleNextButton(const unsigned char optionsCount)
{
    if (OswHal::getInstance()->btnHasGoneDown(BUTTON_1))
    {
        step = (step + 1) % optionsCount;
    }
}

void OswAppAlarm::handleTimeIncrementButton()
{
    if (OswHal::getInstance()->btnHasGoneDown(BUTTON_3))
    {
        switch (step)
        {
        case 0:
            timestamp[step] = (timestamp[step] + 1) % 3;
            break;
        case 1:
            if (timestamp[0] == 1)
            {
                timestamp[step] = (timestamp[step] + 1) % 4;
            }
            else
            {
                timestamp[step] = (timestamp[step] + 1) % 10;
            }
            break;
        case 2:
            timestamp[step] = (timestamp[step] + 1) % 6;
            break;
        case 3:
            timestamp[step] = (timestamp[step] + 1) % 10;
            break;
        case 4:
            state = AlarmState::IDLE;
            notifications = notifierClient.readNotifications();
            step = {};
            timestamp = {};
            clockAppSwitcher->paginationEnable();
            break;
        case 5:
            if (10 * timestamp[0] + timestamp[1] < 24)
            {
                state = AlarmState::DAY_PICKER;
                step = {};
            }
            break;
        }
    }
}

void OswAppAlarm::handleTimeDecrementButton()
{
    if (OswHal::getInstance()->btnHasGoneDown(BUTTON_2))
    {
        switch (step)
        {
        case 0:
            timestamp[step] = (timestamp[step] + 2) % 3;
            break;
        case 1:
            if (timestamp[0] == 1)
            {
                timestamp[step] = (timestamp[step] + 3) % 4;
            }
            else
            {
                timestamp[step] = (timestamp[step] + 9) % 10;
            }
            break;
        case 2:
            timestamp[step] = (timestamp[step] + 5) % 6;
            break;
        case 3:
            timestamp[step] = (timestamp[step] + 9) % 10;
            break;
        }
    }
}

void OswAppAlarm::resetAlarmState()
{
    state = AlarmState::IDLE;
    notifications = notifierClient.readNotifications();
    step = {};
    timestamp = {};
    daysOfWeek = {};
    clockAppSwitcher->paginationEnable();
}

void OswAppAlarm::handleFrequencyIncrementButton()
{
    auto *hal = OswHal::getInstance();
    if (hal->btnHasGoneDown(BUTTON_3))
    {
        switch (step)
        {
        case 7:
            resetAlarmState();
            break;
        case 8:
            notifierClient.createNotification(10 * timestamp[0] + timestamp[1], 10 * timestamp[2] + timestamp[3],
                                              LANG_ALARM, daysOfWeek);
            resetAlarmState();
            break;
        default:
            daysOfWeek[step] ^= 1;
            break;
        }
    }
}

void OswAppAlarm::listAlarms()
{
    auto *hal = OswHal::getInstance();
    auto *ui = OswUI::getInstance();
    const auto colorActive = ui->getDangerColor();
    const auto colorForeground = ui->getForegroundColor();
    const auto colorBackground = ui->getBackgroundColor();

    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextCenterAligned();
    hal->gfx()->setTextCursor((DISP_W / 2), DISP_H * 2 / 12);
    hal->gfx()->print(LANG_ALARM);

    if (state == AlarmState::LIST)
    {
        ui->setTextCursor(BUTTON_1);
        hal->gfx()->print(">");
    }

    ui->setTextCursor(BUTTON_2);
    hal->gfx()->print("-");

    ui->setTextCursor(BUTTON_3);
    if (state == AlarmState::LIST)
    {
        hal->gfx()->print("x");
    }
    else
    {
        hal->gfx()->print("+");
    }

    hal->gfx()->setTextLeftAligned();
    for (size_t i{}; i < notifications.size(); ++i)
    {
        hal->gfx()->setTextSize(2);
        hal->gfx()->setTextCursor(hal->gfx()->getTextOfsetColumns(1.5), DISP_H * (i + 2) / 8);
        hal->gfx()->setTextColor(state == AlarmState::LIST && step == i ? colorActive : colorForeground, colorBackground);
        auto timeToFire = notifications[i].first;
        date::hh_mm_ss time{floor<std::chrono::seconds>(timeToFire - floor<date::days>(timeToFire))};
        auto hours = time.hours().count();
        hal->gfx()->print(hours / 10);
        hal->gfx()->print(hours % 10);
        hal->gfx()->print(":");
        auto minutes = time.minutes().count();
        hal->gfx()->print(minutes / 10);
        hal->gfx()->print(minutes % 10);
        hal->gfx()->setTextSize(1);
        hal->gfx()->setTextCursor(DISP_W / 3 + hal->gfx()->getTextOfsetColumns(1.5), DISP_H * (i + 2) / 8);
        auto myDaysOfWeek = notifications[i].second.getDaysOfWeek();
        if (std::all_of(myDaysOfWeek.begin(), myDaysOfWeek.end(), [](bool x)
                        { return x; }))
        {
            hal->gfx()->print(LANG_DAILY);
        }
        else if (
            std::all_of(myDaysOfWeek.begin() + 1, myDaysOfWeek.begin() + 6, [](bool x)
                        { return x; }) &&
            not myDaysOfWeek[0] && !myDaysOfWeek[6])
        {
            hal->gfx()->print(LANG_WEEKDAYS);
        }
        else if (
            std::all_of(myDaysOfWeek.begin() + 1, myDaysOfWeek.begin() + 6, [](bool x)
                        { return !x; }) &&
            myDaysOfWeek[0] && myDaysOfWeek[6])
        {
            hal->gfx()->print(LANG_WEEKENDS);
        }
        else
        {
            for (size_t j{}; j < myDaysOfWeek.size(); ++j)
            {
                if (myDaysOfWeek[j])
                {
                    switch (j)
                    {
                    case 0:
                        hal->gfx()->print(LANG_SUN);
                        hal->gfx()->print(" ");
                        break;
                    case 1:
                        hal->gfx()->print(LANG_MON);
                        hal->gfx()->print(" ");
                        break;
                    case 2:
                        hal->gfx()->print(LANG_TUE);
                        hal->gfx()->print(" ");
                        break;
                    case 3:
                        hal->gfx()->print(LANG_WED);
                        hal->gfx()->print(" ");
                        break;
                    case 4:
                        hal->gfx()->print(LANG_THU);
                        hal->gfx()->print(" ");
                        break;
                    case 5:
                        hal->gfx()->print(LANG_FRI);
                        hal->gfx()->print(" ");
                        break;
                    case 6:
                        hal->gfx()->print(LANG_SAT);
                        hal->gfx()->print(" ");
                        break;
                    }
                }
            }
        }
    }
}

void OswAppAlarm::loop()
{
    auto *hal = OswHal::getInstance();
    auto *ui = OswUI::getInstance();
    const auto colorActive = ui->getDangerColor();
    const auto colorForeground = ui->getForegroundColor();
    const auto colorBackground = ui->getBackgroundColor();

    switch (state)
    {
    case AlarmState::IDLE:
    {
        if (hal->btnHasGoneDown(BUTTON_2) && !notifications.empty())
        {
            state = AlarmState::LIST;
            notifications = notifierClient.readNotifications();
            clockAppSwitcher->paginationDisable();
        }

        if (hal->btnHasGoneDown(BUTTON_3) && notifications.size() < ALARM_COUNT)
        {
            state = AlarmState::TIME_PICKER;
            clockAppSwitcher->paginationDisable();
        }

        listAlarms();
    }
    break;

    case AlarmState::LIST:
    {
        handleNextButton(notifications.size());

        if (hal->btnHasGoneDown(BUTTON_2))
        {
            notifierClient.deleteNotification(notifications[step].second.getId());
            state = AlarmState::IDLE;
            notifications = notifierClient.readNotifications();
            step = {};
            clockAppSwitcher->paginationEnable();
        }

        if (hal->btnHasGoneDown(BUTTON_3))
        {
            state = AlarmState::IDLE;
            notifications = notifierClient.readNotifications();
            step = {};
            clockAppSwitcher->paginationEnable();
        }

        listAlarms();
    }
    break;

    case AlarmState::TIME_PICKER:
    {
        handleNextButton(6);
        handleTimeIncrementButton();
        handleTimeDecrementButton();

        hal->gfx()->setTextSize(2);
        hal->gfx()->setTextCenterAligned();
        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 2 / 12);
        hal->gfx()->print(LANG_ALARM);

        ui->setTextCursor(BUTTON_1);
        hal->gfx()->print(">");

        ui->setTextCursor(BUTTON_2);
        hal->gfx()->print("-");

        ui->setTextCursor(BUTTON_3);
        hal->gfx()->print("+");

        hal->gfx()->setTextRightAligned();
        hal->gfx()->setTextCursor((DISP_W / 2) - hal->gfx()->getTextOfsetColumns(0.5), DISP_H * 5 / 8);
        hal->gfx()->setTextColor(step == 4 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_CANCEL);

        hal->gfx()->setTextLeftAligned();
        hal->gfx()->setTextCursor((DISP_W / 2) + hal->gfx()->getTextOfsetColumns(0.5), DISP_H * 5 / 8);
        hal->gfx()->setTextColor(step == 5 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_NEXT);

        hal->gfx()->setTextSize(4);
        hal->gfx()->setTextMiddleAligned();
        hal->gfx()->setTextCursor(DISP_W / 2 - 60, DISP_H / 2);
        for (int i{}; i < 4; ++i)
        {
            hal->gfx()->setTextColor(step == i ? colorActive : colorForeground, colorBackground);
            hal->gfx()->print(timestamp[i]);
            if (i == 1)
            {
                ui->resetTextColors();
                hal->gfx()->print(":");
            }
        }
    }
    break;

    case AlarmState::DAY_PICKER:
    {
        handleNextButton(9);
        handleFrequencyIncrementButton();

        hal->gfx()->setTextSize(2);
        hal->gfx()->setTextCenterAligned();
        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 2 / 12);
        hal->gfx()->print(LANG_ALARM);

        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 3 / 12);
        hal->gfx()->setTextColor(step == 0 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_SUNDAY);

        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 4 / 12);
        hal->gfx()->setTextColor(step == 1 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_MONDAY);

        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 5 / 12);
        hal->gfx()->setTextColor(step == 2 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_TUESDAY);

        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 6 / 12);
        hal->gfx()->setTextColor(step == 3 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_WEDNESDAY);

        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 7 / 12);
        hal->gfx()->setTextColor(step == 4 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_THURSDAY);

        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 8 / 12);
        hal->gfx()->setTextColor(step == 5 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_FRIDAY);

        hal->gfx()->setTextCursor(DISP_W / 2, DISP_H * 9 / 12);
        hal->gfx()->setTextColor(step == 6 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_SATURDAY);

        ui->resetTextColors();
        for (size_t i{}; i < daysOfWeek.size(); ++i)
        {
            if (daysOfWeek[i])
            {
                hal->gfx()->setTextCursor(DISP_W / 4, DISP_H * (i + 3) / 12);
                hal->gfx()->print(">");
            }
        }

        hal->gfx()->setTextRightAligned();
        hal->gfx()->setTextCursor((DISP_W / 2) - hal->gfx()->getTextOfsetColumns(0.5), DISP_H * 10 / 12);
        hal->gfx()->setTextColor(step == 7 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_CANCEL);

        hal->gfx()->setTextLeftAligned();
        hal->gfx()->setTextCursor((DISP_W / 2) + hal->gfx()->getTextOfsetColumns(0.5), DISP_H * 10 / 12);
        hal->gfx()->setTextColor(step == 8 ? colorActive : colorForeground, colorBackground);
        hal->gfx()->print(LANG_SAVE);
    }
    break;
    }

    ui->resetTextColors();
}
