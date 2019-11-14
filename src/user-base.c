/*   mate-user-admin 
*   Copyright (C) 2018  zhuyaliang https://github.com/zhuyaliang/
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.

*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "user-base.h"
#include "user-password.h"
#include "user-share.h"
#include "user-info.h"
#include "user-group.h"
#include "user-history.h"
#include "user-language.h"

static void chooser_language_cancel(GtkWidget *widget, 
                                    UserAdmin *ua)
{
    gtk_widget_hide_on_delete(GTK_WIDGET(ua->language_chooser));
}

static void chooser_language_done (GtkWidget *widget,
                                   UserAdmin *ua)
{
    UserInfo  *user;
    const gchar *lang, *account_language;
    gchar *name = NULL;


    user = GetIndexUser(ua->UsersList,gnCurrentUserIndex);
    account_language = act_user_get_language (user->ActUser);
    lang = language_chooser_get_language (LANGUAGE_CHOOSER (ua->language_chooser));
    if (lang) 
    {
        if (g_strcmp0 (lang, account_language) != 0) 
        {
            act_user_set_language (user->ActUser, lang);
        }

        name = mate_get_language_from_locale (lang, NULL);
        gtk_button_set_label(GTK_BUTTON(ua->ButtonLanguage),
                             name);

        g_free (name);
     }

    gtk_widget_hide (GTK_WIDGET (ua->language_chooser));

}    
static void
change_language (GtkButton   *button,
                 UserAdmin   *self)
{
    const gchar *current_language;
    UserInfo    *user;
    const gchar *current_name;

    user = GetIndexUser(self->UsersList,gnCurrentUserIndex);
    current_language = GetUserLang(user->ActUser);
    current_name =  GetRealName(user->ActUser);

    if (self->language_chooser != NULL)
    {
        language_chooser_clear_filter (self->language_chooser);
        language_chooser_set_language (self->language_chooser, NULL);
    }   
    else
    {    
        self->language_chooser = language_chooser_new (current_name);
        g_signal_connect (self->language_chooser->done_button, 
                         "clicked",
                          G_CALLBACK (chooser_language_done), 
                          self);

        g_signal_connect (self->language_chooser->cancel_button, 
                         "clicked",
                          G_CALLBACK (chooser_language_cancel), 
                          self);
    }
    if (current_language && *current_language != '\0')
        language_chooser_set_language (self->language_chooser, current_language);
    gtk_widget_show_all(GTK_WIDGET(self->language_chooser));
}

/******************************************************************************
* Function:            SwitchState 
*        
* Explain: Select auto login,Only one user can choose to log in automatically.
*        
* Input:         
*        
* Output: 
*        
* Author:  zhuyaliang  09/05/2018
******************************************************************************/
static void SwitchState(GtkSwitch *widget,gboolean state,gpointer data)
{
    GSList    *list;
    GSList    *l;
    UserAdmin *ua = (UserAdmin *)data;
    UserInfo  *user;
    ActUserManager *um;
    
    if(Change == 0)
    {        
        user = GetIndexUser(ua->UsersList,gnCurrentUserIndex);
        um =  act_user_manager_get_default ();
        if(state == TRUE)
        {
            list = act_user_manager_list_users (um);
            for (l = list; l != NULL; l = l->next)
            {
                ActUser *u = l->data;
                if (act_user_get_uid (u) != act_user_get_uid (user->ActUser)) 
                {
                    act_user_set_automatic_login (u, FALSE);
                }
            }
            g_slist_free (list);
            act_user_set_automatic_login(user->ActUser,TRUE);
        }
        else
            act_user_set_automatic_login(user->ActUser,FALSE);

    }
}    
/******************************************************************************
* Function:             ChangePass 
*        
* Explain: Modifying the cipher signal.The two state .Change the password 
*          .Set set the new password.
* Input:         
*        
* Output: 
*        
* Author:  zhuyaliang  15/05/2018
******************************************************************************/
static void ChangePass(GtkWidget *widget,gpointer data)
{
    UserAdmin *ua = (UserAdmin *)data;
    CreateNewPass(ua);      //There is no password for the user
}
/******************************************************************************
* Function:             ComboSelectUserType 
*        
* Explain: Select user type signal
*        
* Input:         
*        
* Output: 
*        
* Author:  zhuyaliang  09/05/2018
******************************************************************************/
static void ComboSelectUserType(GtkWidget *widget,gpointer data)
{
    UserAdmin *ua = (UserAdmin *)data;
    UserInfo  *user;
    gint       account_type;

    if( Change ==0 )
    {
        account_type =  gtk_combo_box_get_active (GTK_COMBO_BOX(widget)) ? 
                                                  ACT_USER_ACCOUNT_TYPE_ADMINISTRATOR:
                                                  ACT_USER_ACCOUNT_TYPE_STANDARD;
        user = GetIndexUser(ua->UsersList,gnCurrentUserIndex);
        act_user_set_account_type(user->ActUser,account_type);
    }    
}
/******************************************************************************
* Function:              DisplayUserSetOther 
*        
* Explain: Displays user type and user language and user password, 
*          and automatic login and logon time.
*        
* Input:         
*        
* Output: 
*        
* Author:  zhuyaliang  09/05/2018
******************************************************************************/
void DisplayUserSetOther(GtkWidget *Hbox,UserAdmin *ua)
{
    GtkWidget *table;
    GtkWidget *fixed;
    GtkWidget *ButtonPass;
    GtkWidget *LabelUserType;
    GtkWidget *LabelAutoLogin;
    GtkWidget *LabelLanguage;
    GtkWidget *SwitchLogin;
    GtkWidget *LabelPass;
    GtkWidget *LabelTime;
    GtkWidget *ButtonTime;
    GtkWidget *ComboUser;
    GtkWidget *LabelGroup;
    GtkWidget *ButtonGroup;
    UserInfo  *user;
    char      *lang;
    const char *lang_id;
    g_autofree const char *time = NULL;

    user = GetIndexUser(ua->UsersList,0);
    fixed = gtk_fixed_new();
    gtk_box_pack_start(GTK_BOX(Hbox),fixed ,TRUE, TRUE, 0);
    table = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(table),TRUE);
    gtk_fixed_put(GTK_FIXED(fixed), table, 0, 0);

    /*user type*/
    LabelUserType = gtk_label_new(NULL);
    SetLableFontType(LabelUserType,"gray",11,_("Account Type"));
    gtk_grid_attach(GTK_GRID(table) , LabelUserType , 0 , 0 , 1 , 1);

    /*drop-down select boxes*/
    ComboUser = SetComboUserType(_("Standard"),_("Administrators"));
    ua->ComUserType = ComboUser; 
    gtk_combo_box_set_active(GTK_COMBO_BOX(ComboUser),GetUserType(user->ActUser));
    gtk_grid_attach(GTK_GRID(table) , ComboUser , 1 , 0 , 2 , 1);
    g_signal_connect(G_OBJECT(ComboUser),
                    "changed",
                     G_CALLBACK(ComboSelectUserType),
                     ua);

   /*select language*/ 
    LabelLanguage = gtk_label_new(NULL);
    SetLableFontType(LabelLanguage,"gray",11,_("Language"));
    gtk_grid_attach(GTK_GRID(table) , LabelLanguage , 0 , 1 , 1 , 1);
    lang_id = GetUserLang(user->ActUser);
    if(lang_id == NULL)
    {
        ua->ButtonLanguage = gtk_button_new_with_label(_("No Settings"));
    }    
    else
    {
        lang = mate_get_language_from_locale (lang_id, NULL);
        ua->ButtonLanguage = gtk_button_new_with_label(lang);
		g_free (lang);
    }
    g_signal_connect (ua->ButtonLanguage, 
                     "clicked",
                      G_CALLBACK (change_language),
                      ua);
    gtk_grid_attach(GTK_GRID(table) , ua->ButtonLanguage , 1 , 1 , 2 , 1);
    /*set password*/
    LabelPass = gtk_label_new(NULL);
    SetLableFontType(LabelPass,"gray",11,_("Password"));
    gtk_grid_attach(GTK_GRID(table) , LabelPass , 0 , 2 , 1 , 1);
    ButtonPass = gtk_button_new_with_label(GetPasswordModeText(user->ActUser,
                                           &user->PasswordType));
    ua->ButtonPass = ButtonPass;
    g_signal_connect (ButtonPass, 
                     "clicked",
                      G_CALLBACK (ChangePass),
                      ua);
    gtk_grid_attach(GTK_GRID(table) , ButtonPass , 1 , 2 , 2 , 1);
   
    /*auto login*/
    LabelAutoLogin = gtk_label_new(NULL);
    SetLableFontType(LabelAutoLogin,"gray",11,_("Automatic logon"));
    gtk_grid_attach(GTK_GRID(table) , LabelAutoLogin , 0 , 3 , 1 , 1);
    SwitchLogin = gtk_switch_new();
    ua->SwitchAutoLogin = SwitchLogin;
    gtk_switch_set_state (GTK_SWITCH(SwitchLogin),
                          GetUserType(user->ActUser));
    gtk_grid_attach(GTK_GRID(table) , SwitchLogin , 1 , 3 , 2 , 1);
    g_signal_connect(G_OBJECT(SwitchLogin),
                    "state-set",
                     G_CALLBACK(SwitchState),
                     ua);
    
    /*login time*/
    LabelTime = gtk_label_new(NULL);
    SetLableFontType(LabelTime,"gray",11,_("Login time"));
    gtk_grid_attach(GTK_GRID(table) , LabelTime, 0 , 4 , 1 , 1);
    
    time = GetLoginTimeText(user->ActUser);
    ButtonTime = gtk_button_new_with_label (time);
    ua->ButtonUserTime = ButtonTime;
    gtk_grid_attach(GTK_GRID(table) , ButtonTime, 1 , 4 , 2 , 1);
    g_signal_connect (ButtonTime, 
                     "clicked",
                      G_CALLBACK (ViewLoginHistory),
                      ua);

    /*Group Manage*/
    LabelGroup = gtk_label_new(NULL);
    SetLableFontType(LabelGroup,"gray",11,_("Group Manage"));
    gtk_grid_attach(GTK_GRID(table) , LabelGroup, 0 , 5 , 1 , 1);
  
    ButtonGroup = gtk_button_new_with_label (_("Setting Groups"));
    ua->ButtonUserGroup = ButtonGroup;
    gtk_grid_attach(GTK_GRID(table) , ButtonGroup, 1 , 5 , 2 , 1);
    g_signal_connect (ButtonGroup, 
                     "clicked",
                      G_CALLBACK (UserGroupsManage),
                      ua);

    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
}
