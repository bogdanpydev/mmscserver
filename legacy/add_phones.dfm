object OKRightDlg: TOKRightDlg
  Left = 262
  Top = 184
  BorderStyle = bsDialog
  Caption = 'Add phone numbers'
  ClientHeight = 169
  ClientWidth = 339
  Color = clBtnFace
  ParentFont = True
  FormStyle = fsStayOnTop
  OldCreateOrder = True
  Position = poScreenCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 8
    Top = 8
    Width = 161
    Height = 161
    Shape = bsFrame
  end
  object Label1: TLabel
    Left = 16
    Top = 16
    Width = 15
    Height = 16
    Caption = 'IP'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 16
    Top = 88
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
  object OKBtn: TButton
    Left = 196
    Top = 8
    Width = 75
    Height = 25
    Caption = 'ADD'
    Default = True
    ModalResult = 1
    TabOrder = 0
    OnClick = OKBtnClick
  end
  object CancelBtn: TButton
    Left = 196
    Top = 86
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
    OnClick = CancelBtnClick
  end
  object Edit1: TEdit
    Left = 16
    Top = 32
    Width = 121
    Height = 21
    TabOrder = 2
    OnKeyPress = Edit1KeyPress
  end
  object Edit2: TEdit
    Left = 16
    Top = 112
    Width = 121
    Height = 21
    TabOrder = 3
    OnKeyPress = Edit2KeyPress
  end
end
