#pragma once

/**
 * page_manager.h
 * ─────────────────────────────────────────────────────────────
 *  Button layout:
 *
 *  NEXT  (GPIO 35) → always cycles to next page (all pages)
 *
 *  ┌──────────────┬────────────────────┬──────────────────────┐
 *  │ Page         │ PREV (GPIO 0)      │ ACTION (GPIO 34)     │
 *  ├──────────────┼────────────────────┼──────────────────────┤
 *  │ Clock        │ —                  │ —                    │
 *  │ Calendar     │ previous month     │ next month / today   │
 *  │ Stopwatch    │ reset              │ start / stop         │
 *  │ To-Do        │ scroll up          │ mark done / undone   │
 *  └──────────────┴────────────────────┴──────────────────────┘
 */

#define PAGE_CLOCK      0
#define PAGE_CALENDAR   1
#define PAGE_STOPWATCH  2
#define PAGE_TODO       3
#define PAGE_COUNT      4

extern int g_current_page;

void page_manager_init(void);
void page_next(void);                    // NEXT btn — always cycles page

void page_handle_prev_btn(void);         // PREV btn — context action
void page_handle_action_btn(void);       // ACTION btn — context action
