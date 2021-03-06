/* libvorbisfile_winrt - Ogg Vorbis for Windows Runtime
 * Copyright (C) 2014-2015  Alexander Ovchinnikov
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of copyright holder nor the names of project's
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _OV_WINRT_INTERNAL_H_
#define _OV_WINRT_INTERNAL_H_

#include <robuffer.h>
#include <ppltasks.h>
#include <windows.storage.streams.h>

#define THROW_IF_FAILED(hr) { if (FAILED((hr))) throw Platform::Exception::CreateException((hr)); }


/* get internal IBuffer array */
static
uint8 *get_array(Windows::Storage::Streams::IBuffer^ buffer)
{
	HRESULT hr = S_OK;
	uint8 *pBytes = nullptr;

	Microsoft::WRL::ComPtr<Windows::Storage::Streams::IBufferByteAccess> spBuffAccess;
	Microsoft::WRL::ComPtr<ABI::Windows::Storage::Streams::IBuffer> spBuff = reinterpret_cast<ABI::Windows::Storage::Streams::IBuffer*>(buffer);

	THROW_IF_FAILED(spBuff.As(&spBuffAccess));
	THROW_IF_FAILED(spBuffAccess->Buffer(&pBytes));

	return pBytes;
}


/* convert UTF-8 to String */
static
Platform::String^ string_from_utf8(const char *str, int len = -1)
{
	if (!str) return nullptr;

	int wlen = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
	if (0 == wlen) {
		throw ref new Platform::InvalidArgumentException();
	}

	wchar_t *widestr = new wchar_t[wlen];
	if (0 == MultiByteToWideChar(CP_UTF8, 0, str, len, widestr, wlen)) {
		delete[] widestr;
		throw ref new Platform::InvalidArgumentException();
	}

	Platform::String^ ret = ref new Platform::String(widestr);
	delete[] widestr;
	return ret;
}


template<typename _Ty>
static
typename concurrency::details::_TaskTypeFromParam<_Ty>::_Type perform_synchronously(_Ty param)
{
	concurrency::task<typename concurrency::details::_TaskTypeFromParam<_Ty>::_Type> task = concurrency::create_task(param);
	concurrency::event synchronizer;
	task.then([&](typename concurrency::details::_TaskTypeFromParam<_Ty>::_Type) {
		synchronizer.set();
	}, concurrency::task_continuation_context::use_arbitrary());
	synchronizer.wait();
	return task.get();
}


#endif
