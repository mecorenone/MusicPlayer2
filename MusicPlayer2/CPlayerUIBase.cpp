#include "stdafx.h"
#include "CPlayerUIBase.h"


CPlayerUIBase::CPlayerUIBase(UIData& ui_data)
	:m_ui_data(ui_data)
{
	m_popup_menu.LoadMenu(IDR_LYRIC_POPUP_MENU);	//װ�ظ���Ҽ��˵�
	m_main_popup_menu.LoadMenu(IDR_MAIN_POPUP_MENU);

	m_font_time.CreatePointFont(80, CCommon::LoadText(IDS_DEFAULT_FONT));
}


CPlayerUIBase::~CPlayerUIBase()
{
}

void CPlayerUIBase::SetToolTip(CToolTipCtrl * pToolTip)
{
	m_tool_tip = pToolTip;
}

void CPlayerUIBase::Init(CDC * pDC)
{
	m_pDC = pDC;
	m_draw.Create(m_pDC, theApp.m_pMainWnd);
	m_pLayout = std::make_shared<SLayoutData>();
}

void CPlayerUIBase::DrawInfo(bool reset)
{
	if (m_first_draw)
	{
		AddToolTips();
	}
	m_first_draw = false;
}

void CPlayerUIBase::ClearInfo()
{
	PreDrawInfo();
	m_pDC->FillSolidRect(m_draw_rect, GetSysColor(COLOR_BTNFACE));
}

void CPlayerUIBase::LButtonDown(CPoint point)
{
	for (auto& btn : m_buttons)
	{
		if(btn.second.rect.PtInRect(point) != FALSE)
			btn.second.pressed = true;
	}
}

void CPlayerUIBase::RButtonUp(CPoint point)
{
	if (m_buttons[BTN_VOLUME].rect.PtInRect(point) == FALSE)
		m_show_volume_adj = false;

	CPoint point1;		//����һ������ȷ�����λ�õ�λ��  
	GetCursorPos(&point1);	//��ȡ��ǰ����λ�ã��Ա�ʹ�ò˵����Ը����꣬��λ������Ļ���Ͻǵ�Ϊԭ�㣬point���Կͻ������Ͻ�Ϊԭ��
	if (m_buttons[BTN_REPETEMODE].rect.PtInRect(point))		//����ڡ�ѭ��ģʽ���ľ��������ڵ������Ҽ����򵯳���ѭ��ģʽ�����Ӳ˵�
	{
		CMenu* pMenu = m_main_popup_menu.GetSubMenu(0)->GetSubMenu(1);
		if (pMenu != NULL)
			pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, theApp.m_pMainWnd);
	}

}

void CPlayerUIBase::MouseMove(CPoint point)
{
	for (auto& btn : m_buttons)
	{
		btn.second.hover = (btn.second.rect.PtInRect(point) != FALSE);
	}

	//���ָ�������ʱ��ʾ��λ�����ּ���
	if (m_buttons[BTN_PROGRESS].hover)
	{
		__int64 song_pos;
		song_pos = static_cast<__int64>(point.x - m_buttons[BTN_PROGRESS].rect.left) * CPlayer::GetInstance().GetSongLength() / m_buttons[BTN_PROGRESS].rect.Width();
		Time song_pos_time;
		song_pos_time.int2time(static_cast<int>(song_pos));
		CString str;
		static int last_sec{};
		if (last_sec != song_pos_time.sec)		//ֻ�����ָ��λ�ö�Ӧ�������仯�˲Ÿ��������ʾ
		{
			str.Format(CCommon::LoadText(IDS_SEEK_TO_MINUTE_SECOND), song_pos_time.min, song_pos_time.sec);
			UpdateMouseToolTip(BTN_PROGRESS, str);
			last_sec = song_pos_time.sec;
		}
	}

}

void CPlayerUIBase::LButtonUp(CPoint point)
{
	for (auto& btn : m_buttons)
	{
		//if (btn.second.rect.PtInRect(point) != FALSE)
		//{
			//btn.second.hover = false;
			btn.second.pressed = false;
		//}
	}

	if (m_buttons[BTN_REPETEMODE].rect.PtInRect(point))	//����ˡ�ѭ��ģʽ��ʱ������ѭ��ģʽ
	{
		CPlayer::GetInstance().SetRepeatMode();
		UpdateRepeatModeToolTip();
	}

	if (!m_show_volume_adj)		//���������ʾ����������ť�����������������ʾ����������ť
		m_show_volume_adj = (m_buttons[BTN_VOLUME].rect.PtInRect(point) != FALSE);
	else		//����Ѿ���ʾ������������ť��������������ʱ��������������ť����ʾ
		m_show_volume_adj = (m_volume_up_rect.PtInRect(point) || m_volume_down_rect.PtInRect(point));

	if (m_show_volume_adj && m_volume_up_rect.PtInRect(point))	//�������������ť�е�������ʱ��������
	{
		CPlayer::GetInstance().MusicControl(Command::VOLUME_UP, theApp.m_nc_setting_data.volum_step);
	}
	if (m_show_volume_adj && m_volume_down_rect.PtInRect(point))	//�������������ť�е�������ʱ������С
	{
		CPlayer::GetInstance().MusicControl(Command::VOLUME_DOWN, theApp.m_nc_setting_data.volum_step);
	}

	if (m_buttons[BTN_SKIN].rect.PtInRect(point))
	{
		m_buttons[BTN_SKIN].hover = false;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_SWITCH_UI);
	}

	if (m_buttons[BTN_EQ].rect.PtInRect(point))
	{
		m_buttons[BTN_EQ].hover = false;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_EQUALIZER);
	}

	if (m_buttons[BTN_SETTING].rect.PtInRect(point))
	{
		m_buttons[BTN_SETTING].hover = false;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_OPTION_SETTINGS);
	}

	if (m_buttons[BTN_MINI].rect.PtInRect(point))
	{
		m_buttons[BTN_MINI].hover = false;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_MINI_MODE);
	}

	if (m_buttons[BTN_INFO].rect.PtInRect(point))
	{
		m_buttons[BTN_INFO].hover = false;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_SONG_INFO);
	}

	if (m_buttons[BTN_STOP].rect.PtInRect(point))
	{
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_STOP);
	}

	if (m_buttons[BTN_PREVIOUS].rect.PtInRect(point))
	{
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_PREVIOUS);
	}

	if (m_buttons[BTN_PLAY_PAUSE].rect.PtInRect(point))
	{
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_PLAY_PAUSE);
	}

	if (m_buttons[BTN_NEXT].rect.PtInRect(point))
	{
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_NEXT);
	}

	if (m_buttons[BTN_PROGRESS].rect.PtInRect(point))
	{
		int ckick_pos = point.x - m_buttons[BTN_PROGRESS].rect.left;
		double progress = static_cast<double>(ckick_pos) / m_buttons[BTN_PROGRESS].rect.Width();
		CPlayer::GetInstance().SeekTo(progress);

	}

	if (m_buttons[BTN_SHOW_PLAYLIST].rect.PtInRect(point))
	{
		m_buttons[BTN_SHOW_PLAYLIST].hover = false;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_SHOW_PLAYLIST);
	}

	if (m_buttons[BTN_SELECT_FOLDER].rect.PtInRect(point))
	{
		m_buttons[BTN_SELECT_FOLDER].hover = false;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_SET_PATH);
	}

}

void CPlayerUIBase::OnSizeRedraw(int cx, int cy)
{
	CRect redraw_rect;
	if (!m_ui_data.m_narrow_mode)	//����ͨ����ģʽ��
	{
		if (cx < m_ui_data.client_width)	//���������ȱ�խ��
		{
			//���½���ͼ�����Ҳ�����ľ����������Ϊ�Ի��򱳾�ɫ
			redraw_rect = m_draw_rect;
			if (m_ui_data.show_playlist)
			{
				redraw_rect.left = cx / 2/* - m_pLayout->margin*/;
				redraw_rect.right = m_ui_data.client_width / 2 + m_pLayout->margin;
			}
			else
			{
				redraw_rect.left = m_draw_rect.right - m_pLayout->margin;
				redraw_rect.right = m_draw_rect.right + m_pLayout->margin;
			}
			m_pDC->FillSolidRect(redraw_rect, GetSysColor(COLOR_BTNFACE));
		}
		if (cy < m_ui_data.client_height)	//�������߶ȱ�С��
		{
			//���½���ͼ�����·�����ľ����������Ϊ�Ի��򱳾�ɫ
			redraw_rect = m_draw_rect;
			redraw_rect.top = cy - m_pLayout->margin;
			redraw_rect.bottom = cy;
			m_pDC->FillSolidRect(redraw_rect, GetSysColor(COLOR_BTNFACE));
		}
	}
	else if (m_ui_data.m_narrow_mode)	//��խ����ģʽ��
	{
		if (cx < m_ui_data.client_width)		//������ȱ�խ��
		{
			//���½���ͼ�����Ҳ�����ľ����������Ϊ�Ի��򱳾�ɫ
			redraw_rect = m_draw_rect;
			redraw_rect.left = cx - m_pLayout->margin;
			redraw_rect.right = cx;
			m_pDC->FillSolidRect(redraw_rect, GetSysColor(COLOR_BTNFACE));
		}
		if (cy < m_ui_data.client_height)	//�������߶ȱ�С��
		{
			if (!m_ui_data.show_playlist)
			{
				//���½���ͼ�����·�����ľ����������Ϊ�Ի��򱳾�ɫ
				redraw_rect = m_draw_rect;
				redraw_rect.top = cy - m_pLayout->margin;
				redraw_rect.bottom = cy;
				m_pDC->FillSolidRect(redraw_rect, GetSysColor(COLOR_BTNFACE));
			}
		}
	}
}

void CPlayerUIBase::UpdateRepeatModeToolTip()
{
	SetRepeatModeToolTipText();
	UpdateMouseToolTip(BTN_REPETEMODE, m_repeat_mode_tip);
}

void CPlayerUIBase::UpdateSongInfoToolTip()
{
	SetSongInfoToolTipText();
	UpdateMouseToolTip(BTN_INFO, m_info_tip);
}

void CPlayerUIBase::UpdatePlayPauseButtonTip()
{
	if (CPlayer::GetInstance().IsPlaying() && !CPlayer::GetInstance().IsError())
		UpdateMouseToolTip(BTN_PLAY_PAUSE, CCommon::LoadText(IDS_PAUSE));
	else
		UpdateMouseToolTip(BTN_PLAY_PAUSE, CCommon::LoadText(IDS_PLAY));
}

bool CPlayerUIBase::SetCursor()
{
	if (m_buttons[BTN_PROGRESS].hover)
	{
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(32649)));
		return true;
	}
	return false;
}

void CPlayerUIBase::PreDrawInfo()
{
	//������ɫ
	m_colors = CPlayerUIHelper::GetUIColors(theApp.m_app_setting_data.theme_color);

	//if (m_repeat_mode_tip.IsEmpty())
	SetRepeatModeToolTipText();
	SetSongInfoToolTipText();

	//�����ı��߶�
	m_pDC->SelectObject(&m_ui_data.lyric_font);
	m_lyric_text_height = m_pDC->GetTextExtent(L"��").cy;	//���ݵ�ǰ���������ü����ı��ĸ߶�

	//���û��Ƶľ�������
	SetDrawRect();
}

void CPlayerUIBase::SetDrawRect()
{
	if (!m_ui_data.m_narrow_mode)
	{
		if (m_ui_data.show_playlist)
		{
			m_draw_rect = CRect{ CPoint{m_pLayout->margin, m_pLayout->margin},
		   CPoint{m_ui_data.client_width / 2/* - m_pLayout->margin*/, m_ui_data.client_height - m_pLayout->margin} };
		}
		else
		{
			m_draw_rect = CRect{ CPoint{m_pLayout->margin, m_pLayout->margin},
		    CPoint{m_ui_data.client_width - m_pLayout->margin, m_ui_data.client_height - m_pLayout->margin} };
		}
	}
	else
	{
		if (m_ui_data.show_playlist)
		{
			m_draw_rect = CRect{ CPoint{ m_pLayout->margin, m_pLayout->margin},
			CSize{ m_ui_data.client_width - 2 * m_pLayout->margin, m_pLayout->info_height - 2 * m_pLayout->margin } };
		}
		else
		{
			m_draw_rect = CRect{ CPoint{ m_pLayout->margin, m_pLayout->margin},
			CPoint{m_ui_data.client_width - m_pLayout->margin, m_ui_data.client_height - m_pLayout->margin} };
		}
	}
}

void CPlayerUIBase::DrawLyricTextMultiLine(CRect lyric_area, bool midi_lyric)
{
	int lyric_height = m_lyric_text_height + theApp.m_app_setting_data.lyric_line_space;			//�ı��߶ȼ����м��
	int lyric_height2 = lyric_height * 2 + theApp.m_app_setting_data.lyric_line_space;		//��������ĸ�ʸ߶�

	m_draw.SetFont(&m_ui_data.lyric_font);
	if (midi_lyric)
	{
		wstring current_lyric{ CPlayer::GetInstance().GetMidiLyric() };
		m_draw.DrawWindowText(lyric_area, current_lyric.c_str(), m_colors.color_text, Alignment::CENTER, false, true);
	}
	else if (CPlayer::GetInstance().m_Lyrics.IsEmpty())
	{
		m_draw.DrawWindowText(lyric_area, CCommon::LoadText(IDS_NO_LYRIC_INFO), m_colors.color_text_2, Alignment::CENTER);
	}
	else
	{
		//CRect arect{ lyric_area };		//һ�и�ʵľ�������
		//arect.bottom = arect.top + lyric_height;
		//vector<CRect> rects(CPlayer::GetInstance().m_Lyrics.GetLyricCount() + 1, arect);
		//Ϊÿһ���ʴ���һ�����Σ�������������
		vector<CRect> rects;
		int lyric_count = CPlayer::GetInstance().m_Lyrics.GetLyricCount() + 1;		//��ȡ������������ڵ�һ�и����Ҫ��ʾ���⣬��������Ҫ+1��
		for (int i{}; i < lyric_count; i++)
		{
			CRect arect{ lyric_area };
			if (!CPlayer::GetInstance().m_Lyrics.GetLyric(i).translate.empty() && m_ui_data.show_translate)
				arect.bottom = arect.top + lyric_height2;
			else
				arect.bottom = arect.top + lyric_height;
			rects.push_back(arect);
		}
		int center_pos = (lyric_area.top + lyric_area.bottom) / 2;		//������������y����
		Time time{ CPlayer::GetInstance().GetCurrentPosition() };		//��ǰ����ʱ��
		int lyric_index = CPlayer::GetInstance().m_Lyrics.GetLyricIndex(time) + 1;		//��ǰ��ʵ���ţ���ʵĵ�һ��GetLyricIndex���ص���0��������ʾʱ��һ����Ҫ��ʾ���⣬��������Ҫ+1��
		int progress = CPlayer::GetInstance().m_Lyrics.GetLyricProgress(time);		//��ǰ��ʽ��ȣ���ΧΪ0~1000��
		int y_progress;			//��ǰ�����y���ϵĽ���
		if (!CPlayer::GetInstance().m_Lyrics.GetLyric(lyric_index).translate.empty() && m_ui_data.show_translate)
			y_progress = progress * lyric_height2 / 1000;
		else
			y_progress = progress * lyric_height / 1000;
		//int start_pos = center_pos - y_progress - (lyric_index + 1)*lyric_height;		//��1���ʵ���ʼy����
		//�����1���ʵ���ʼy����
		//���ڵ�ǰ�����Ҫ��ʾ�ڸ�����������λ�ã���˴�����λ�ÿ�ʼ����ȥ��ǰ�����Y���ϵĽ���
		//�����μ�ȥ֮ǰÿһ���ʵĸ߶ȣ����õ��˵�һ���ʵ���ʼλ��
		int start_pos;
		start_pos = center_pos - y_progress;
		for (int i{ lyric_index - 1 }; i >= 0; i--)
		{
			if (!CPlayer::GetInstance().m_Lyrics.GetLyric(i).translate.empty() && m_ui_data.show_translate)
				start_pos -= lyric_height2;
			else
				start_pos -= lyric_height;
		}

		//���λ���ÿһ����
		for (int i{}; i < rects.size(); i++)
		{
			//����ÿһ���ʵ�λ��
			if (i == 0)
				rects[i].MoveToY(start_pos);
			else
				rects[i].MoveToY(rects[i - 1].bottom);
			//���Ƹ���ı�
			if (!(rects[i] & lyric_area).IsRectEmpty())		//ֻ�е�һ���ʵľ�������͸������ľ����н���ʱ���Ż��Ƹ��
			{
				//���ø���ı��ͷ����ı��ľ�������
				CRect rect_text{ rects[i] };
				CRect rect_translate;
				if (!CPlayer::GetInstance().m_Lyrics.GetLyric(i).translate.empty() && m_ui_data.show_translate)
				{
					rect_text.MoveToY(rect_text.top + theApp.m_app_setting_data.lyric_line_space);
					rect_text.bottom = rect_text.top + m_lyric_text_height;
					rect_translate = rect_text;
					rect_translate.MoveToY(rect_text.bottom + theApp.m_app_setting_data.lyric_line_space);
				}

				if (i == lyric_index)		//�������ڲ��ŵĸ��
				{
					//���Ƹ���ı�
					m_draw.SetFont(&m_ui_data.lyric_font);
					if (theApp.m_lyric_setting_data.lyric_karaoke_disp)
						m_draw.DrawWindowText(rect_text, CPlayer::GetInstance().m_Lyrics.GetLyric(i).text.c_str(), m_colors.color_text, m_colors.color_text_2, progress, true, true);
					else
						m_draw.DrawWindowText(rect_text, CPlayer::GetInstance().m_Lyrics.GetLyric(i).text.c_str(), m_colors.color_text, m_colors.color_text, progress, true, true);
					//���Ʒ����ı�
					if (!CPlayer::GetInstance().m_Lyrics.GetLyric(i).translate.empty() && m_ui_data.show_translate)
					{
						m_draw.SetFont(&m_ui_data.lyric_translate_font);
						m_draw.DrawWindowText(rect_translate, CPlayer::GetInstance().m_Lyrics.GetLyric(i).translate.c_str(), m_colors.color_text, m_colors.color_text, progress, true, true);
					}
				}
				else		//���Ʒ����ڲ��ŵĸ��
				{
					//���Ƹ���ı�
					m_draw.SetFont(&m_ui_data.lyric_font);
					m_draw.DrawWindowText(rect_text, CPlayer::GetInstance().m_Lyrics.GetLyric(i).text.c_str(), m_colors.color_text_2, Alignment::CENTER, true);
					//���Ʒ����ı�
					if (!CPlayer::GetInstance().m_Lyrics.GetLyric(i).translate.empty() && m_ui_data.show_translate)
					{
						m_draw.SetFont(&m_ui_data.lyric_translate_font);
						m_draw.DrawWindowText(rect_translate, CPlayer::GetInstance().m_Lyrics.GetLyric(i).translate.c_str(), m_colors.color_text_2, Alignment::CENTER, true);
					}
				}
			}
		}
	}

}

void CPlayerUIBase::DrawLyricTextSingleLine(CRect rect, bool midi_lyric)
{
	m_draw.SetFont(&m_ui_data.lyric_font);
	if (midi_lyric)
	{
		wstring current_lyric{ CPlayer::GetInstance().GetMidiLyric() };
		m_draw.DrawWindowText(rect, current_lyric.c_str(), m_colors.color_text, Alignment::CENTER, false, true);
	}
	else if (CPlayer::GetInstance().m_Lyrics.IsEmpty())
	{
		m_draw.DrawWindowText(rect, CCommon::LoadText(IDS_NO_LYRIC_INFO), m_colors.color_text_2, Alignment::CENTER);
	}
	else
	{
		CRect lyric_rect = rect;
		CLyrics::Lyric current_lyric{ CPlayer::GetInstance().m_Lyrics.GetLyric(Time(CPlayer::GetInstance().GetCurrentPosition()), 0) };	//��ȡ�����
		if (current_lyric.text.empty())		//�����ǰ���Ϊ�հף�����ʾΪʡ�Ժ�
			current_lyric.text = CCommon::LoadText(IDS_DEFAULT_LYRIC_TEXT);
		int progress{ CPlayer::GetInstance().m_Lyrics.GetLyricProgress(Time(CPlayer::GetInstance().GetCurrentPosition())) };		//��ȡ��ǰ��ʽ��ȣ���ΧΪ0~1000��

		if ((!CPlayer::GetInstance().m_Lyrics.IsTranslated() || !m_ui_data.show_translate) && rect.Height() > static_cast<int>(m_lyric_text_height*1.73))
		{
			wstring next_lyric_text = CPlayer::GetInstance().m_Lyrics.GetLyric(Time(CPlayer::GetInstance().GetCurrentPosition()), 1).text;
			if (next_lyric_text.empty())
				next_lyric_text = CCommon::LoadText(IDS_DEFAULT_LYRIC_TEXT);
			DrawLyricDoubleLine(lyric_rect, current_lyric.text.c_str(), next_lyric_text.c_str(), progress);
		}
		else
		{
			if (m_ui_data.show_translate && !current_lyric.translate.empty() && rect.Height() > static_cast<int>(m_lyric_text_height*1.73))
			{
				lyric_rect.bottom = lyric_rect.top + rect.Height() / 2;
				CRect translate_rect = lyric_rect;
				translate_rect.MoveToY(lyric_rect.bottom);

				m_draw.SetFont(&m_ui_data.lyric_translate_font);
				m_draw.DrawWindowText(translate_rect, current_lyric.translate.c_str(), m_colors.color_text, m_colors.color_text, progress, true, true);
			}

			m_draw.SetFont(&m_ui_data.lyric_font);
			if (theApp.m_lyric_setting_data.lyric_karaoke_disp)
				m_draw.DrawWindowText(lyric_rect, current_lyric.text.c_str(), m_colors.color_text, m_colors.color_text_2, progress, true, true);
			else
				m_draw.DrawWindowText(lyric_rect, current_lyric.text.c_str(), m_colors.color_text, m_colors.color_text, progress, true, true);
		}

		m_draw.SetFont(theApp.m_pMainWnd->GetFont());
	}

}

void CPlayerUIBase::DrawSongInfo(CRect rect, bool reset)
{
	wchar_t buff[64];
	if (CPlayer::GetInstance().m_loading)
	{
		static CDrawCommon::ScrollInfo scroll_info0;
		CString info;
		info = CCommon::LoadTextFormat(IDS_PLAYLIST_INIT_INFO, { CPlayer::GetInstance().GetSongNum(), CPlayer::GetInstance().m_thread_info.process_percent });
		m_draw.DrawScrollText(rect, info, m_colors.color_text, theApp.DPI(1.5), false, scroll_info0, reset);
	}
	else
	{
		//���Ʋ���״̬
		CString play_state_str = CPlayer::GetInstance().GetPlayingState().c_str();
		CRect rc_tmp{ rect };
		rc_tmp.right = rc_tmp.left + m_draw.GetDC()->GetTextExtent(play_state_str).cx - theApp.DPI(4);
		m_draw.DrawWindowText(rc_tmp, play_state_str, m_colors.color_text_lable);

		//���Ƹ������
		rc_tmp.MoveToX(rc_tmp.right);
		rc_tmp.right = rc_tmp.left + theApp.DPI(30);
		swprintf_s(buff, sizeof(buff) / 2, L"%.3d", CPlayer::GetInstance().GetIndex() + 1);
		m_draw.DrawWindowText(rc_tmp, buff, m_colors.color_text_2);

		//�����ļ���
		rc_tmp.MoveToX(rc_tmp.right);
		rc_tmp.right = rect.right;
		static CDrawCommon::ScrollInfo scroll_info1;
		m_draw.DrawScrollText(rc_tmp, CPlayer::GetInstance().GetFileName().c_str(), m_colors.color_text, theApp.DPI(1.5), false, scroll_info1, reset);
	}
}

void CPlayerUIBase::DrawToolBar(bool draw_background, CRect rect, bool draw_translate_button, UIData* pUIData)
{
	//���Ʊ���
	BYTE alpha;
	if (theApp.m_app_setting_data.dark_mode)
		alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency) * 2 / 3;
	else
		alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency);

	if (draw_background)
		m_draw.FillAlphaRect(rect, m_colors.color_control_bar_back, alpha);
	else
		m_draw.FillRect(rect, m_colors.color_control_bar_back);

	CRect rc_tmp = rect;

	//����ѭ��ģʽ
	rc_tmp.right = rect.left + rect.Height();
	CRect rc_repeat_mode = rc_tmp;
	rc_repeat_mode.DeflateRect(theApp.DPI(2), theApp.DPI(2));
	m_draw.SetDrawArea(rc_repeat_mode);

	if (draw_background)
		alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency);
	else
		alpha = 255;
	if (m_buttons[BTN_REPETEMODE].pressed && m_buttons[BTN_REPETEMODE].hover)
		m_draw.FillAlphaRect(rc_repeat_mode, m_colors.color_button_pressed, alpha);
	else if (m_buttons[BTN_REPETEMODE].hover)
		m_draw.FillAlphaRect(rc_repeat_mode, m_colors.color_text_2, alpha);

	//else if (!theApp.m_app_setting_data.dark_mode)
	//	m_draw.FillAlphaRect(rc_repeat_mode, m_colors.color_button_back, alpha);

	m_buttons[BTN_REPETEMODE].rect = DrawAreaToClient(rc_repeat_mode, m_draw_rect);

	rc_repeat_mode = rc_tmp;
	rc_repeat_mode.DeflateRect(theApp.DPI(4), theApp.DPI(4));

	switch (CPlayer::GetInstance().GetRepeatMode())
	{
	case RepeatMode::RM_PLAY_ORDER:
		m_draw.DrawIcon(theApp.m_play_oder_icon.GetIcon(!theApp.m_app_setting_data.dark_mode), rc_repeat_mode.TopLeft(), rc_repeat_mode.Size());
		break;
	case RepeatMode::RM_LOOP_PLAYLIST:
		m_draw.DrawIcon(theApp.m_loop_playlist_icon.GetIcon(!theApp.m_app_setting_data.dark_mode), rc_repeat_mode.TopLeft(), rc_repeat_mode.Size());
		break;
	case RepeatMode::RM_LOOP_TRACK:
		m_draw.DrawIcon(theApp.m_loop_track_icon.GetIcon(!theApp.m_app_setting_data.dark_mode), rc_repeat_mode.TopLeft(), rc_repeat_mode.Size());
		break;
	case RepeatMode::RM_PLAY_SHUFFLE:
		m_draw.DrawIcon(theApp.m_play_shuffle_icon.GetIcon(!theApp.m_app_setting_data.dark_mode), rc_repeat_mode.TopLeft(), rc_repeat_mode.Size());
		break;
	}
	

	//�������ð�ť
	rc_tmp.MoveToX(rc_tmp.right);
	DrawUIButton(rc_tmp, m_buttons[BTN_SETTING], theApp.m_setting_icon, draw_background);

	//���ƾ�������ť
	rc_tmp.MoveToX(rc_tmp.right);
	DrawUIButton(rc_tmp, m_buttons[BTN_EQ], theApp.m_eq_icon, draw_background);

	//�����л����水ť
	rc_tmp.MoveToX(rc_tmp.right);
	DrawUIButton(rc_tmp, m_buttons[BTN_SKIN], theApp.m_skin_icon, draw_background);

	//��������ģʽ��ť
	if (rect.Width() >= theApp.DPI(190))
	{
		rc_tmp.MoveToX(rc_tmp.right);
		DrawUIButton(rc_tmp, m_buttons[BTN_MINI], theApp.m_mini_icon, draw_background);
	}
	else
	{
		m_buttons[BTN_MINI].rect = CRect();
	}

	//������Ŀ��Ϣ��ť
	if (rect.Width() >= theApp.DPI(214))
	{
		rc_tmp.MoveToX(rc_tmp.right);
		DrawUIButton(rc_tmp, m_buttons[BTN_INFO], theApp.m_info_icon, draw_background);
	}
	else
	{
		m_buttons[BTN_INFO].rect = CRect();
	}

	//���Ʒ��밴ť
	if (draw_translate_button && rect.Width() >= theApp.DPI(238))
	{
		rc_tmp.MoveToX(rc_tmp.right);
		CRect translate_rect = rc_tmp;
		translate_rect.DeflateRect(theApp.DPI(2), theApp.DPI(2));
		m_buttons[BTN_TRANSLATE].enable = CPlayer::GetInstance().m_Lyrics.IsTranslated();
		if (m_buttons[BTN_TRANSLATE].enable)
		{
			BYTE alpha;
			if (draw_background)
				alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency) * 2 / 3;
			else
				alpha = 255;
			if (m_buttons[BTN_TRANSLATE].hover)
				m_draw.FillAlphaRect(translate_rect, m_colors.color_text_2, alpha);
			else if (pUIData!=nullptr && pUIData->show_translate)
				m_draw.FillAlphaRect(translate_rect, m_colors.color_button_back, alpha);
			m_draw.DrawWindowText(translate_rect, CCommon::LoadText(IDS_TRAS), m_colors.color_text, Alignment::CENTER);
		}
		else
		{
			m_draw.DrawWindowText(translate_rect, CCommon::LoadText(IDS_TRAS), GRAY(200), Alignment::CENTER);
		}
		m_buttons[BTN_TRANSLATE].rect = DrawAreaToClient(translate_rect, m_draw_rect);

	}
	else
	{
		m_buttons[BTN_TRANSLATE].rect = CRect();
	}

	rc_tmp.left = rc_tmp.right = rect.right;

	//��ʾ<<<<
	if (rect.Width() >= theApp.DPI(289))
	{
		int progress;
		Time time{ CPlayer::GetInstance().GetCurrentPosition() };
		if (CPlayer::GetInstance().IsMidi())
		{
			////progress = (CPlayer::GetInstance().GetMidiInfo().midi_position % 16 + 1) *1000 / 16;
			//if (CPlayer::GetInstance().GetMidiInfo().tempo == 0)
			//	progress = 0;
			//else
			//	progress = (time.time2int() * 1000 / CPlayer::GetInstance().GetMidiInfo().tempo % 4 + 1) * 250;
			progress = (CPlayer::GetInstance().GetMidiInfo().midi_position % 4 + 1) * 250;
		}
		else
		{
			progress = (time.sec % 4 * 1000 + time.msec) / 4;
		}
		rc_tmp.right = rc_tmp.left;
		rc_tmp.left = rc_tmp.right - theApp.DPI(44);
		m_draw.DrawWindowText(rc_tmp, _T("<<<<"), m_colors.color_text, m_colors.color_text_2, progress, false);
	}


	//��ʾ����
	wchar_t buff[64];
	rc_tmp.right = rc_tmp.left;
	rc_tmp.left = rc_tmp.right - theApp.DPI(72);
	swprintf_s(buff, CCommon::LoadText(IDS_VOLUME, _T(": %d%%")), CPlayer::GetInstance().GetVolume());
	if (m_buttons[BTN_VOLUME].hover)		//���ָ����������ʱ��������һ����ɫ��ʾ
		m_draw.DrawWindowText(rc_tmp, buff, m_colors.color_text_heighlight);
	else
		m_draw.DrawWindowText(rc_tmp, buff, m_colors.color_text);
	//��������������ť��λ��
	m_buttons[BTN_VOLUME].rect = DrawAreaToClient(rc_tmp, m_draw_rect);
	m_buttons[BTN_VOLUME].rect.DeflateRect(0, theApp.DPI(4));
	m_buttons[BTN_VOLUME].rect.right -= theApp.DPI(12);
	m_volume_down_rect = m_buttons[BTN_VOLUME].rect;
	m_volume_down_rect.bottom += theApp.DPI(4);
	m_volume_down_rect.MoveToY(m_buttons[BTN_VOLUME].rect.bottom);
	m_volume_down_rect.right = m_buttons[BTN_VOLUME].rect.left + m_buttons[BTN_VOLUME].rect.Width() / 2;
	m_volume_up_rect = m_volume_down_rect;
	m_volume_up_rect.MoveToX(m_volume_down_rect.right);


}

CRect CPlayerUIBase::DrawAreaToClient(CRect rect, CRect draw_area)
{
	rect.MoveToXY(rect.left + draw_area.left, rect.top + draw_area.top);
	return rect;
}

CRect CPlayerUIBase::ClientAreaToDraw(CRect rect, CRect draw_area)
{
	rect.MoveToXY(rect.left - draw_area.left, rect.top - draw_area.top);
	return rect;
}

void CPlayerUIBase::DrawLyricDoubleLine(CRect rect, LPCTSTR lyric, LPCTSTR next_lyric, int progress)
{
	m_draw.SetFont(&m_ui_data.lyric_font);
	static bool swap;
	static int last_progress;
	if (last_progress > progress)		//�����ǰ�ĸ�ʽ��ȱ��ϴε�С��˵������л�������һ��
	{
		swap = !swap;
	}
	last_progress = progress;


	CRect up_rect{ rect }, down_rect{ rect };		//�ϰ벿�ֺ��°벿�ָ�ʵľ�������
	up_rect.bottom = up_rect.top + (up_rect.Height() / 2);
	down_rect.top = down_rect.bottom - (down_rect.Height() / 2);
	//������һ���ʵ��ı�������Ҫ�Ŀ��ȣ��Ӷ�ʵ����һ�и���Ҷ���
	m_draw.GetDC()->SelectObject(&m_ui_data.lyric_font);
	int width;
	if (!swap)
		width = m_draw.GetDC()->GetTextExtent(next_lyric).cx;
	else
		width = m_draw.GetDC()->GetTextExtent(lyric).cx;
	if (width < rect.Width())
		down_rect.left = down_rect.right - width;

	COLORREF color1, color2;
	if (theApp.m_lyric_setting_data.lyric_karaoke_disp)
	{
		color1 = m_colors.color_text;
		color2 = m_colors.color_text_2;
	}
	else
	{
		color1 = color2 = m_colors.color_text;
	}

	if (!swap)
	{
		m_draw.DrawWindowText(up_rect, lyric, color1, color2, progress, false);
		m_draw.DrawWindowText(down_rect, next_lyric, m_colors.color_text_2);
	}
	else
	{
		m_draw.DrawWindowText(up_rect, next_lyric, m_colors.color_text_2);
		m_draw.DrawWindowText(down_rect, lyric, color1, color2, progress, false);
	}

}

void CPlayerUIBase::DrawUIButton(CRect rect, UIButton & btn, const IconRes& icon, bool draw_background)
{
	CRect rc_tmp = rect;
	rc_tmp.DeflateRect(theApp.DPI(2), theApp.DPI(2));
	m_draw.SetDrawArea(rc_tmp);

	BYTE alpha;
	if (draw_background)
		alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency) * 2 / 3;
	else
		alpha = 255;
	if(btn.pressed && btn.hover)
		m_draw.FillAlphaRect(rc_tmp, m_colors.color_button_pressed, alpha);
	else if (btn.hover)
		m_draw.FillAlphaRect(rc_tmp, m_colors.color_text_2, alpha);

	//else if (!theApp.m_app_setting_data.dark_mode)
	//	m_draw.FillAlphaRect(rc_tmp, m_colors.color_button_back, alpha);

	btn.rect = DrawAreaToClient(rc_tmp, m_draw_rect);

	rc_tmp = rect;
	rc_tmp.DeflateRect(theApp.DPI(4), theApp.DPI(4));
	const HICON& hIcon = icon.GetIcon(!theApp.m_app_setting_data.dark_mode);
	m_draw.DrawIcon(hIcon, rc_tmp.TopLeft(), rc_tmp.Size());

}

void CPlayerUIBase::DrawControlButton(CRect rect, UIButton & btn, const IconRes & icon, bool draw_background)
{
	CRect rc_tmp = rect;
	m_draw.SetDrawArea(rc_tmp);

	BYTE alpha;
	if (draw_background)
		alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency) * 2 / 3;
	else
		alpha = 255;
	if (btn.pressed && btn.hover)
		m_draw.FillAlphaRect(rc_tmp, m_colors.color_button_pressed, alpha);
	else if (btn.hover)
		m_draw.FillAlphaRect(rc_tmp, m_colors.color_text_2, alpha);

	//else if (!theApp.m_app_setting_data.dark_mode)
	//	m_draw.FillAlphaRect(rc_tmp, m_colors.color_button_back, alpha);

	btn.rect = DrawAreaToClient(rc_tmp, m_draw_rect);

	rc_tmp = rect;
	//ʹͼ���ھ����о���
	rc_tmp.left = rect.left + (rect.Width() - icon.GetSize().cx) / 2;
	rc_tmp.right = rc_tmp.left + icon.GetSize().cx;
	rc_tmp.top = rect.top + (rect.Height() - icon.GetSize().cy) / 2;
	rc_tmp.bottom = rc_tmp.top + icon.GetSize().cy;

	//const HICON& hIcon = icon.GetIcon(!theApp.m_app_setting_data.dark_mode);
	m_draw.DrawIcon(icon.GetIcon(), rc_tmp.TopLeft(), rc_tmp.Size());
}

void CPlayerUIBase::SetRepeatModeToolTipText()
{
	m_repeat_mode_tip = CCommon::LoadText(IDS_REPEAT_MODE, _T(": "));
	switch (CPlayer::GetInstance().GetRepeatMode())
	{
	case RepeatMode::RM_PLAY_ORDER:
		m_repeat_mode_tip += CCommon::LoadText(IDS_PLAY_ODER);
		break;
	case RepeatMode::RM_LOOP_PLAYLIST:
		m_repeat_mode_tip += CCommon::LoadText(IDS_LOOP_PLAYLIST);
		break;
	case RepeatMode::RM_LOOP_TRACK:
		m_repeat_mode_tip += CCommon::LoadText(IDS_LOOP_TRACK);
		break;
	case RepeatMode::RM_PLAY_SHUFFLE:
		m_repeat_mode_tip += CCommon::LoadText(IDS_PLAY_SHUFFLE);
		break;
	}
}

void CPlayerUIBase::SetSongInfoToolTipText()
{
	const SongInfo& songInfo = CPlayer::GetInstance().GetCurrentSongInfo();
	
	m_info_tip = CCommon::LoadText(IDS_SONG_INFO, _T("\r\n"));

	m_info_tip += CCommon::LoadText(IDS_TITLE, _T(": "));
	m_info_tip += songInfo.title.c_str();
	m_info_tip += _T("\r\n");

	m_info_tip += CCommon::LoadText(IDS_ARTIST, _T(": "));
	m_info_tip += songInfo.artist.c_str();
	m_info_tip += _T("\r\n");

	m_info_tip += CCommon::LoadText(IDS_ALBUM, _T(": "));
	m_info_tip += songInfo.album.c_str();
	//m_info_tip += _T("\r\n");

	//m_info_tip += CCommon::LoadText(IDS_BITRATE, _T(": "));
	//CString strTmp;
	//strTmp.Format(_T("%d kbps"), songInfo.bitrate);
	//m_info_tip += strTmp;
}

bool CPlayerUIBase::DrawNarrowMode()
{
	if (!m_ui_data.show_playlist)
		return false;
	else
		return m_ui_data.m_narrow_mode;
}

void CPlayerUIBase::DrawVolumnAdjBtn(bool draw_background)
{
	if (m_show_volume_adj)
	{
		CRect volume_down_rect = ClientAreaToDraw(m_volume_down_rect, m_draw_rect);
		CRect volume_up_rect = ClientAreaToDraw(m_volume_up_rect, m_draw_rect);

		if (draw_background)
		{
			m_draw.FillAlphaRect(volume_down_rect, m_colors.color_text_2, ALPHA_CHG(theApp.m_app_setting_data.background_transparency));
			m_draw.FillAlphaRect(volume_up_rect, m_colors.color_text_2, ALPHA_CHG(theApp.m_app_setting_data.background_transparency));
		}
		else
		{
			m_draw.FillRect(volume_down_rect, m_colors.color_text_2);
			m_draw.FillRect(volume_up_rect, m_colors.color_text_2);
		}
		m_draw.DrawWindowText(volume_down_rect, L"-", ColorTable::WHITE, Alignment::CENTER);
		m_draw.DrawWindowText(volume_up_rect, L"+", ColorTable::WHITE, Alignment::CENTER);
	}
}

void CPlayerUIBase::DrawControlBar(CRect rect, bool draw_background)
{
	bool progress_on_top = rect.Width() < theApp.DPI(300);
	const int progress_height = theApp.DPI(4);
	CRect progress_rect;
	if (progress_on_top)
	{
		progress_rect = rect;
		progress_rect.bottom = progress_rect.top + theApp.DPI(8);
		DrawProgressBar(progress_rect, draw_background);
		rect.top = progress_rect.bottom;
	}

	//���Ʋ��ſ��ư�ť
	const int btn_width = theApp.DPI(36);
	const int btn_height = rect.Height();

	CRect rc_btn{ rect.TopLeft(), CSize(btn_width, btn_height) };
	DrawControlButton(rc_btn, m_buttons[BTN_STOP], theApp.m_stop_icon_l, draw_background);

	rc_btn.MoveToX(rc_btn.right);
	DrawControlButton(rc_btn, m_buttons[BTN_PREVIOUS], theApp.m_previous_icon_l, draw_background);

	rc_btn.MoveToX(rc_btn.right);
	IconRes& paly_pause_icon = CPlayer::GetInstance().IsPlaying() ? theApp.m_pause_icon_l : theApp.m_play_icon_l;
	DrawControlButton(rc_btn, m_buttons[BTN_PLAY_PAUSE], paly_pause_icon, draw_background);

	rc_btn.MoveToX(rc_btn.right);
	DrawControlButton(rc_btn, m_buttons[BTN_NEXT], theApp.m_next_icon_l, draw_background);

	int progressbar_left = rc_btn.right + m_pLayout->margin;

	//�����Ҳఴť
	const int btn_side = theApp.DPI(24);
	rc_btn.right = rect.right;
	rc_btn.left = rc_btn.right - btn_side;
	rc_btn.top = rect.top + (rect.Height() - btn_side) / 2;
	rc_btn.bottom = rc_btn.top + btn_side;
	DrawUIButton(rc_btn, m_buttons[BTN_SHOW_PLAYLIST], theApp.m_show_playlist_icon, draw_background);

	rc_btn.MoveToX(rc_btn.left - btn_side);
	DrawUIButton(rc_btn, m_buttons[BTN_SELECT_FOLDER], theApp.m_select_folder_icon, draw_background);

	if (!progress_on_top)
	{
		progress_rect = rect;
		progress_rect.left = progressbar_left;
		progress_rect.right = rc_btn.left - m_pLayout->margin;
		DrawProgressBar(progress_rect, draw_background);
	}
}

void CPlayerUIBase::DrawProgressBar(CRect rect, bool draw_background)
{
	//���Ʋ���ʱ��
	CRect rc_time = rect;
	wstring strTime = CPlayer::GetInstance().GetTimeString();

	m_pDC->SelectObject(&m_font_time);
	CSize strSize = m_pDC->GetTextExtent(strTime.c_str());
	rc_time.left = rc_time.right - strSize.cx;
	m_draw.SetFont(&m_font_time);
	//rc_time.InflateRect(0, theApp.DPI(2));
	rc_time.top -= theApp.DPI(1);
	m_draw.DrawWindowText(rc_time, strTime.c_str(), m_colors.color_text);

	//���ƽ�����
	const int progress_height = theApp.DPI(4);
	CRect progress_rect = rect;
	progress_rect.right = rc_time.left - m_pLayout->margin;
	progress_rect.top = rect.top + (rect.Height() - progress_height) / 2;
	progress_rect.bottom = progress_rect.top + progress_height;

	if (draw_background)
		m_draw.FillAlphaRect(progress_rect, m_colors.color_spectrum_back, ALPHA_CHG(theApp.m_app_setting_data.background_transparency) * 2 / 3);
	else
		m_draw.FillRect(progress_rect, m_colors.color_spectrum_back);

	m_buttons[BTN_PROGRESS].rect = DrawAreaToClient(progress_rect, m_draw_rect);
	m_buttons[BTN_PROGRESS].rect.InflateRect(0, theApp.DPI(2));

	double progress = static_cast<double>(CPlayer::GetInstance().GetCurrentPosition()) / CPlayer::GetInstance().GetSongLength();
	progress_rect.right = progress_rect.left + static_cast<int>(progress * progress_rect.Width());
	if (progress_rect.right > progress_rect.left)
		m_draw.FillRect(progress_rect, m_colors.color_spectrum);
}

//void CPlayerUIBase::AddMouseToolTip(BtnKey btn, LPCTSTR str)
//{
//	m_tool_tip->AddTool(theApp.m_pMainWnd, str, m_buttons[btn].rect, btn + 1);
//}
//
//void CPlayerUIBase::UpdateMouseToolTip(BtnKey btn, LPCTSTR str)
//{
//	m_tool_tip->UpdateTipText(str, theApp.m_pMainWnd, btn + 1);
//}

void CPlayerUIBase::AddToolTips()
{
	AddMouseToolTip(BTN_REPETEMODE, m_repeat_mode_tip);
	AddMouseToolTip(BTN_TRANSLATE, CCommon::LoadText(IDS_SHOW_LYRIC_TRANSLATION));
	AddMouseToolTip(BTN_VOLUME, CCommon::LoadText(IDS_MOUSE_WHEEL_ADJUST_VOLUME));
	AddMouseToolTip(BTN_SKIN, CCommon::LoadText(IDS_SWITCH_UI));
	AddMouseToolTip(BTN_EQ, CCommon::LoadText(IDS_SOUND_EFFECT_SETTING));
	AddMouseToolTip(BTN_SETTING, CCommon::LoadText(IDS_SETTINGS));
	AddMouseToolTip(BTN_MINI, CCommon::LoadText(IDS_MINI_MODE));
	AddMouseToolTip(BTN_INFO, m_info_tip);
	AddMouseToolTip(BTN_STOP, CCommon::LoadText(IDS_STOP));
	AddMouseToolTip(BTN_PREVIOUS, CCommon::LoadText(IDS_PREVIOUS));
	AddMouseToolTip(BTN_PLAY_PAUSE, CPlayer::GetInstance().IsPlaying() ? CCommon::LoadText(IDS_PAUSE) : CCommon::LoadText(IDS_PLAY));
	AddMouseToolTip(BTN_NEXT, CCommon::LoadText(IDS_NEXT));
	AddMouseToolTip(BTN_PROGRESS, CCommon::LoadText(IDS_SEEK_TO));
	AddMouseToolTip(BTN_SHOW_PLAYLIST, CCommon::LoadText(IDS_SHOW_HIDE_PLAYLIST));
	AddMouseToolTip(BTN_SELECT_FOLDER, CCommon::LoadText(IDS_SELECT_FOLDER));
}
