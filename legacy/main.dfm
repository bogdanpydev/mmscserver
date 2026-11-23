object Form1: TForm1
  Left = 450
  Top = 137
  Width = 846
  Height = 654
  Caption = 'MMSC Server'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object TabbedNotebook1: TTabbedNotebook
    Left = 0
    Top = 0
    Width = 833
    Height = 617
    PageIndex = 5
    TabFont.Charset = DEFAULT_CHARSET
    TabFont.Color = clBtnText
    TabFont.Height = -11
    TabFont.Name = 'MS Sans Serif'
    TabFont.Style = []
    TabOrder = 0
    object TTabPage
      Left = 4
      Top = 24
      Caption = 'MMS notification indication'
      object Label3: TLabel
        Left = 448
        Top = 64
        Width = 135
        Height = 16
        Caption = 'MMS in pending list'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object Label5: TLabel
        Left = 744
        Top = 8
        Width = 64
        Height = 13
        Caption = 'Manual mode'
      end
      object Label18: TLabel
        Left = 672
        Top = 64
        Width = 69
        Height = 16
        Caption = 'Sender IP'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object Label19: TLabel
        Left = 472
        Top = 312
        Width = 82
        Height = 16
        Caption = 'Receiver IP'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object GroupBox1: TGroupBox
        Left = 16
        Top = 8
        Width = 377
        Height = 369
        Caption = 'Message parameters'
        TabOrder = 0
        object Label1: TLabel
          Left = 8
          Top = 24
          Width = 70
          Height = 13
          Caption = 'Message Type'
        end
        object Label2: TLabel
          Left = 8
          Top = 56
          Width = 35
          Height = 13
          Caption = 'Version'
        end
        object Label4: TLabel
          Left = 8
          Top = 288
          Width = 81
          Height = 13
          Caption = 'Content Location'
        end
        object Label10: TLabel
          Left = 8
          Top = 96
          Width = 70
          Height = 13
          Caption = 'Transaction ID'
        end
        object Label11: TLabel
          Left = 8
          Top = 128
          Width = 70
          Height = 13
          Caption = 'Message class'
        end
        object Label12: TLabel
          Left = 8
          Top = 168
          Width = 64
          Height = 13
          Caption = 'Message size'
        end
        object Label13: TLabel
          Left = 8
          Top = 216
          Width = 56
          Height = 13
          Caption = 'MMS Expiry'
        end
        object Label15: TLabel
          Left = 8
          Top = 256
          Width = 23
          Height = 13
          Caption = 'From'
        end
        object ComboBox1: TComboBox
          Left = 112
          Top = 24
          Width = 129
          Height = 21
          Enabled = False
          ItemHeight = 13
          ItemIndex = 0
          TabOrder = 0
          Text = 'Notification indication'
          Items.Strings = (
            'Notification indication')
        end
        object ComboBox2: TComboBox
          Left = 112
          Top = 56
          Width = 129
          Height = 21
          ItemHeight = 13
          TabOrder = 1
          Items.Strings = (
            '1.0'
            '1.1'
            '1.2'
            '1.3')
        end
        object Edit1: TEdit
          Left = 8
          Top = 304
          Width = 353
          Height = 21
          Enabled = False
          TabOrder = 2
        end
        object Edit6: TEdit
          Left = 112
          Top = 96
          Width = 137
          Height = 21
          TabOrder = 3
        end
        object ComboBox3: TComboBox
          Left = 112
          Top = 128
          Width = 137
          Height = 21
          ItemHeight = 13
          TabOrder = 4
          Items.Strings = (
            'Personal'
            'Advertisement'
            'Informational'
            'Auto')
        end
        object Edit7: TEdit
          Left = 112
          Top = 168
          Width = 81
          Height = 21
          ReadOnly = True
          TabOrder = 5
        end
        object Edit8: TEdit
          Left = 112
          Top = 208
          Width = 81
          Height = 21
          TabOrder = 6
        end
        object Edit10: TEdit
          Left = 112
          Top = 256
          Width = 129
          Height = 21
          TabOrder = 7
        end
      end
      object GroupBox2: TGroupBox
        Left = 16
        Top = 380
        Width = 377
        Height = 201
        Caption = 'WSP headers'
        TabOrder = 1
        object Label6: TLabel
          Left = 8
          Top = 24
          Width = 92
          Height = 13
          Caption = 'Push application ID'
          Enabled = False
        end
        object Label7: TLabel
          Left = 8
          Top = 64
          Width = 78
          Height = 13
          Caption = 'PDU type (push)'
          Enabled = False
        end
        object Label8: TLabel
          Left = 8
          Top = 96
          Width = 67
          Height = 13
          Caption = 'Header length'
          Enabled = False
        end
        object Label9: TLabel
          Left = 8
          Top = 128
          Width = 85
          Height = 13
          Caption = 'PDU content type'
          Enabled = False
        end
        object Label14: TLabel
          Left = 8
          Top = 168
          Width = 82
          Height = 13
          Caption = 'Encoding version'
          Enabled = False
        end
        object Edit2: TEdit
          Left = 128
          Top = 24
          Width = 81
          Height = 21
          Enabled = False
          ReadOnly = True
          TabOrder = 0
        end
        object Edit3: TEdit
          Left = 128
          Top = 64
          Width = 81
          Height = 21
          Enabled = False
          TabOrder = 1
        end
        object Edit5: TEdit
          Left = 128
          Top = 128
          Width = 177
          Height = 21
          Enabled = False
          TabOrder = 2
        end
        object Edit9: TEdit
          Left = 128
          Top = 168
          Width = 81
          Height = 21
          Enabled = False
          TabOrder = 3
        end
        object Edit4: TEdit
          Left = 128
          Top = 96
          Width = 81
          Height = 21
          Enabled = False
          ReadOnly = True
          TabOrder = 4
        end
      end
      object ListBox1: TListBox
        Left = 424
        Top = 88
        Width = 193
        Height = 209
        ItemHeight = 13
        TabOrder = 2
      end
      object CheckBox1: TCheckBox
        Left = 720
        Top = 8
        Width = 17
        Height = 17
        Caption = 'CheckBox1'
        TabOrder = 3
        OnClick = CheckBox1Click
      end
      object Button1: TButton
        Left = 632
        Top = 548
        Width = 185
        Height = 33
        Caption = 'Send Notification'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 4
        OnClick = Button1Click
      end
      object ListBox4: TListBox
        Left = 624
        Top = 88
        Width = 193
        Height = 209
        ItemHeight = 13
        TabOrder = 5
      end
      object ListBox5: TListBox
        Left = 424
        Top = 336
        Width = 193
        Height = 209
        ItemHeight = 13
        TabOrder = 6
      end
      object ListBox7: TListBox
        Left = 632
        Top = 336
        Width = 81
        Height = 209
        ItemHeight = 13
        TabOrder = 7
      end
    end
    object TTabPage
      Left = 4
      Top = 24
      Caption = 'MMS retrieve conf'
      object Label38: TLabel
        Left = 744
        Top = 8
        Width = 64
        Height = 13
        Caption = 'Manual mode'
      end
      object GroupBox3: TGroupBox
        Left = 16
        Top = 16
        Width = 321
        Height = 489
        Caption = 'Request response'
        TabOrder = 0
        object Label20: TLabel
          Left = 8
          Top = 32
          Width = 70
          Height = 13
          Caption = 'Message Type'
        end
        object Label21: TLabel
          Left = 8
          Top = 64
          Width = 70
          Height = 13
          Caption = 'Transaction ID'
        end
        object Label22: TLabel
          Left = 8
          Top = 96
          Width = 63
          Height = 13
          Caption = 'MMS Version'
        end
        object Label23: TLabel
          Left = 8
          Top = 128
          Width = 57
          Height = 13
          Caption = 'Message ID'
        end
        object Label24: TLabel
          Left = 8
          Top = 160
          Width = 23
          Height = 13
          Caption = 'Date'
        end
        object Label25: TLabel
          Left = 8
          Top = 192
          Width = 23
          Height = 13
          Caption = 'From'
        end
        object Label26: TLabel
          Left = 8
          Top = 224
          Width = 78
          Height = 13
          Caption = 'Previous sent by'
        end
        object Label27: TLabel
          Left = 8
          Top = 256
          Width = 13
          Height = 13
          Caption = 'To'
        end
        object Label28: TLabel
          Left = 8
          Top = 288
          Width = 14
          Height = 13
          Caption = 'CC'
        end
        object Label29: TLabel
          Left = 8
          Top = 320
          Width = 36
          Height = 13
          Caption = 'Subject'
        end
        object Label30: TLabel
          Left = 8
          Top = 352
          Width = 70
          Height = 13
          Caption = 'Message class'
        end
        object Label31: TLabel
          Left = 8
          Top = 384
          Width = 31
          Height = 13
          Caption = 'Priority'
        end
        object Label32: TLabel
          Left = 8
          Top = 416
          Width = 73
          Height = 13
          Caption = 'Delivery Report'
        end
        object Label33: TLabel
          Left = 8
          Top = 456
          Width = 56
          Height = 13
          Caption = 'Read report'
        end
        object Edit11: TEdit
          Left = 112
          Top = 32
          Width = 121
          Height = 21
          ReadOnly = True
          TabOrder = 0
          Text = 'M-retrieve-conf'
        end
        object Edit12: TEdit
          Left = 112
          Top = 64
          Width = 121
          Height = 21
          TabOrder = 1
          Text = 'Transaction-ID-1234567'
        end
        object ComboBox4: TComboBox
          Left = 112
          Top = 96
          Width = 121
          Height = 21
          ItemHeight = 13
          ItemIndex = 0
          TabOrder = 2
          Text = '1.0'
          Items.Strings = (
            '1.0'
            '1.1'
            '1.2'
            '1.3')
        end
        object Edit13: TEdit
          Left = 112
          Top = 128
          Width = 121
          Height = 21
          TabOrder = 3
        end
        object Edit14: TEdit
          Left = 112
          Top = 160
          Width = 121
          Height = 21
          TabOrder = 4
        end
        object Edit15: TEdit
          Left = 112
          Top = 192
          Width = 121
          Height = 21
          TabOrder = 5
        end
        object Edit16: TEdit
          Left = 112
          Top = 224
          Width = 121
          Height = 21
          TabOrder = 6
        end
        object Edit17: TEdit
          Left = 112
          Top = 256
          Width = 121
          Height = 21
          TabOrder = 7
        end
        object Edit18: TEdit
          Left = 112
          Top = 288
          Width = 121
          Height = 21
          TabOrder = 8
        end
        object Edit19: TEdit
          Left = 112
          Top = 320
          Width = 121
          Height = 21
          TabOrder = 9
        end
        object ComboBox5: TComboBox
          Left = 112
          Top = 416
          Width = 121
          Height = 21
          ItemHeight = 13
          TabOrder = 10
        end
        object ComboBox6: TComboBox
          Left = 112
          Top = 456
          Width = 121
          Height = 21
          ItemHeight = 13
          TabOrder = 11
        end
        object ComboBox7: TComboBox
          Left = 112
          Top = 384
          Width = 121
          Height = 21
          ItemHeight = 13
          TabOrder = 12
        end
        object ComboBox8: TComboBox
          Left = 112
          Top = 352
          Width = 121
          Height = 21
          ItemHeight = 13
          TabOrder = 13
        end
      end
      object GroupBox4: TGroupBox
        Left = 352
        Top = 16
        Width = 329
        Height = 217
        Caption = 'Defferal response'
        TabOrder = 1
        object Label34: TLabel
          Left = 16
          Top = 24
          Width = 66
          Height = 13
          Caption = 'Message type'
          Enabled = False
        end
        object Label35: TLabel
          Left = 16
          Top = 72
          Width = 70
          Height = 13
          Caption = 'Transaction ID'
        end
        object Label36: TLabel
          Left = 16
          Top = 120
          Width = 62
          Height = 13
          Caption = 'MMS version'
        end
        object Label37: TLabel
          Left = 16
          Top = 168
          Width = 71
          Height = 13
          Caption = 'Report allowed'
        end
        object Edit20: TEdit
          Left = 120
          Top = 24
          Width = 161
          Height = 21
          Enabled = False
          TabOrder = 0
          Text = 'M-acknowledge-ind'
        end
        object Edit21: TEdit
          Left = 120
          Top = 72
          Width = 161
          Height = 21
          TabOrder = 1
        end
        object ComboBox9: TComboBox
          Left = 120
          Top = 120
          Width = 161
          Height = 21
          ItemHeight = 13
          TabOrder = 2
          Items.Strings = (
            '1.0'
            '1.1'
            '1.2'
            '1.3')
        end
        object ComboBox10: TComboBox
          Left = 120
          Top = 168
          Width = 161
          Height = 21
          ItemHeight = 13
          TabOrder = 3
          Items.Strings = (
            'Yes'
            'No')
        end
      end
      object CheckBox2: TCheckBox
        Left = 720
        Top = 8
        Width = 17
        Height = 17
        Caption = 'CheckBox2'
        TabOrder = 2
        OnClick = CheckBox2Click
      end
    end
    object TTabPage
      Left = 4
      Top = 24
      Caption = 'Delivery and Read report'
      object Label39: TLabel
        Left = 744
        Top = 8
        Width = 35
        Height = 13
        Caption = 'Manual'
      end
      object GroupBox5: TGroupBox
        Left = 8
        Top = 24
        Width = 345
        Height = 345
        Caption = 'Delivery Report'
        TabOrder = 0
        object Label40: TLabel
          Left = 16
          Top = 24
          Width = 66
          Height = 13
          Caption = 'Message type'
          Enabled = False
        end
        object Label41: TLabel
          Left = 16
          Top = 64
          Width = 62
          Height = 13
          Caption = 'MMS version'
        end
        object Label42: TLabel
          Left = 16
          Top = 104
          Width = 57
          Height = 13
          Caption = 'Message ID'
        end
        object Label43: TLabel
          Left = 16
          Top = 144
          Width = 13
          Height = 13
          Caption = 'To'
        end
        object Label44: TLabel
          Left = 16
          Top = 184
          Width = 23
          Height = 13
          Caption = 'Date'
        end
        object Label45: TLabel
          Left = 16
          Top = 224
          Width = 30
          Height = 13
          Caption = 'Status'
        end
        object Edit22: TEdit
          Left = 104
          Top = 24
          Width = 105
          Height = 21
          Enabled = False
          TabOrder = 0
          Text = 'M-delivery.ind'
        end
        object ComboBox11: TComboBox
          Left = 104
          Top = 64
          Width = 105
          Height = 21
          ItemHeight = 13
          TabOrder = 1
          Items.Strings = (
            '1.0'
            '1.1'
            '1.2'
            '1.3')
        end
        object Edit23: TEdit
          Left = 104
          Top = 104
          Width = 105
          Height = 21
          TabOrder = 2
        end
        object Edit24: TEdit
          Left = 104
          Top = 144
          Width = 105
          Height = 21
          TabOrder = 3
        end
        object Edit25: TEdit
          Left = 104
          Top = 184
          Width = 105
          Height = 21
          TabOrder = 4
        end
        object ComboBox12: TComboBox
          Left = 104
          Top = 224
          Width = 105
          Height = 21
          ItemHeight = 13
          TabOrder = 5
          Items.Strings = (
            'Expired'
            'Retrieved'
            'Rejected'
            'Forwarded'
            'Indeterminate')
        end
      end
      object GroupBox6: TGroupBox
        Left = 376
        Top = 24
        Width = 329
        Height = 345
        Caption = 'Read report'
        TabOrder = 1
        object Label46: TLabel
          Left = 16
          Top = 24
          Width = 66
          Height = 13
          Caption = 'Message type'
          Enabled = False
        end
        object Label47: TLabel
          Left = 16
          Top = 80
          Width = 35
          Height = 13
          Caption = 'Version'
        end
        object Label48: TLabel
          Left = 16
          Top = 128
          Width = 57
          Height = 13
          Caption = 'Message ID'
        end
        object Label49: TLabel
          Left = 16
          Top = 176
          Width = 13
          Height = 13
          Caption = 'To'
        end
        object Label50: TLabel
          Left = 16
          Top = 224
          Width = 23
          Height = 13
          Caption = 'From'
        end
        object Label51: TLabel
          Left = 16
          Top = 264
          Width = 23
          Height = 13
          Caption = 'Date'
        end
        object Label52: TLabel
          Left = 16
          Top = 312
          Width = 56
          Height = 13
          Caption = 'Read report'
        end
        object Edit26: TEdit
          Left = 120
          Top = 24
          Width = 161
          Height = 21
          Enabled = False
          TabOrder = 0
          Text = 'M-read-orig-ind'
        end
        object Edit28: TEdit
          Left = 120
          Top = 128
          Width = 161
          Height = 21
          TabOrder = 1
        end
        object ComboBox13: TComboBox
          Left = 120
          Top = 80
          Width = 161
          Height = 21
          ItemHeight = 13
          TabOrder = 2
          Items.Strings = (
            '1.0'
            '1.1'
            '1.2'
            '1.3')
        end
        object Edit27: TEdit
          Left = 120
          Top = 176
          Width = 161
          Height = 21
          TabOrder = 3
        end
        object Edit29: TEdit
          Left = 120
          Top = 216
          Width = 161
          Height = 21
          TabOrder = 4
        end
        object Edit30: TEdit
          Left = 120
          Top = 264
          Width = 161
          Height = 21
          TabOrder = 5
        end
        object ComboBox14: TComboBox
          Left = 120
          Top = 304
          Width = 161
          Height = 21
          ItemHeight = 13
          TabOrder = 6
          Items.Strings = (
            'Read'
            'Deleted without being read')
        end
      end
      object CheckBox3: TCheckBox
        Left = 720
        Top = 8
        Width = 17
        Height = 17
        Caption = 'CheckBox3'
        TabOrder = 2
        OnClick = CheckBox3Click
      end
    end
    object TTabPage
      Left = 4
      Top = 24
      Caption = 'Phone number association '
      object Label16: TLabel
        Left = 104
        Top = 16
        Width = 104
        Height = 16
        Caption = 'COMPUTER IP'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object Label17: TLabel
        Left = 360
        Top = 16
        Width = 123
        Height = 16
        Caption = 'PHONE NUMBER'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object ListBox2: TListBox
        Left = 24
        Top = 40
        Width = 265
        Height = 497
        ItemHeight = 13
        TabOrder = 0
        OnClick = ListBox2Click
      end
      object ListBox3: TListBox
        Left = 288
        Top = 40
        Width = 265
        Height = 497
        ItemHeight = 13
        TabOrder = 1
        OnClick = ListBox3Click
      end
      object Button2: TButton
        Left = 584
        Top = 40
        Width = 209
        Height = 33
        Caption = 'ADD'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 2
        OnClick = Button2Click
      end
      object Button3: TButton
        Left = 584
        Top = 112
        Width = 209
        Height = 33
        Caption = 'Delete'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 3
        OnClick = Button3Click
      end
    end
    object TTabPage
      Left = 4
      Top = 24
      Caption = 'History'
      object ListBox6: TListBox
        Left = 16
        Top = 56
        Width = 793
        Height = 505
        Color = clCream
        ItemHeight = 13
        TabOrder = 0
      end
      object CheckBox7: TCheckBox
        Left = 680
        Top = 24
        Width = 105
        Height = 17
        Caption = 'Enabled'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 1
        OnClick = CheckBox7Click
      end
    end
    object TTabPage
      Left = 4
      Top = 24
      Caption = 'Server Features'
      object GroupBox7: TGroupBox
        Left = 8
        Top = 16
        Width = 345
        Height = 329
        Caption = 'Features'
        TabOrder = 0
        object CheckBox4: TCheckBox
          Left = 16
          Top = 24
          Width = 121
          Height = 25
          Caption = 'Logging'
          TabOrder = 0
        end
        object CheckBox5: TCheckBox
          Left = 16
          Top = 64
          Width = 121
          Height = 17
          Caption = 'Delivery report'
          TabOrder = 1
        end
        object CheckBox6: TCheckBox
          Left = 16
          Top = 96
          Width = 97
          Height = 17
          Caption = 'Read report'
          TabOrder = 2
        end
        object CheckBox8: TCheckBox
          Left = 16
          Top = 128
          Width = 209
          Height = 17
          Caption = 'Delete the messages after sending'
          TabOrder = 3
        end
      end
      object GroupBox8: TGroupBox
        Left = 384
        Top = 16
        Width = 313
        Height = 329
        Caption = 'Configuration'
        TabOrder = 1
        object RadioButton1: TRadioButton
          Left = 24
          Top = 32
          Width = 169
          Height = 17
          Caption = 'Send notifications via sms'
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = RadioButton1Click
        end
        object RadioButton2: TRadioButton
          Left = 24
          Top = 56
          Width = 169
          Height = 17
          Caption = 'Send notifications via WAP'
          TabOrder = 1
          OnClick = RadioButton2Click
        end
        object CheckBox9: TCheckBox
          Left = 24
          Top = 96
          Width = 193
          Height = 41
          Caption = 'Port number for Telephony plugin'
          Enabled = False
          TabOrder = 2
        end
        object Edit31: TEdit
          Left = 224
          Top = 104
          Width = 49
          Height = 21
          Enabled = False
          TabOrder = 3
          Text = '2950'
        end
        object CheckBox10: TCheckBox
          Left = 24
          Top = 152
          Width = 193
          Height = 17
          Caption = 'Default timing for sending the mms'
          TabOrder = 4
          OnClick = CheckBox10Click
        end
        object Edit32: TEdit
          Left = 224
          Top = 152
          Width = 49
          Height = 21
          Enabled = False
          TabOrder = 5
          OnChange = Edit32Change
        end
      end
    end
  end
  object ServerSocket1: TServerSocket
    Active = True
    Port = 80
    ServerType = stNonBlocking
    ThreadCacheSize = 15
    OnClientConnect = ServerSocket1ClientConnect
    OnClientDisconnect = ServerSocket1ClientDisconnect
    OnClientRead = ServerSocket1ClientRead
    OnClientError = ServerSocket1ClientError
    Left = 296
    Top = 88
  end
  object ServerSocket2: TServerSocket
    Active = True
    Port = 8002
    ServerType = stNonBlocking
    OnClientConnect = ServerSocket2ClientConnect
    OnClientDisconnect = ServerSocket2ClientDisconnect
    OnClientRead = ServerSocket2ClientRead
    OnClientError = ServerSocket2ClientError
    Left = 296
    Top = 144
  end
  object Timer1: TTimer
    Enabled = False
    OnTimer = Timer1Timer
    Left = 296
    Top = 248
  end
  object Timer2: TTimer
    Enabled = False
    OnTimer = Timer2Timer
    Left = 296
    Top = 288
  end
  object UdpSocket1: TUdpSocket
    RemotePort = '2948'
    Left = 296
    Top = 184
  end
  object ClientSocket1: TClientSocket
    Active = False
    ClientType = ctNonBlocking
    Port = 2950
    OnConnect = ClientSocket1Connect
    OnDisconnect = ClientSocket1Disconnect
    OnError = ClientSocket1Error
    Left = 344
    Top = 88
  end
  object Timer3: TTimer
    Enabled = False
    Interval = 10
    OnTimer = Timer3Timer
    Left = 296
    Top = 328
  end
end
