/* libvorbisfile_winrt - Ogg Vorbis for Windows Runtime
 * Copyright (C) 2014  Alexander Ovchinnikov
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

#include "vorbisfile.h"
#include <windows.h>

/* convert UTF-8 to String */
static
Platform::String^ string_from_utf8(const char *str, int len = -1)
{
	if (!str) return nullptr;
	if (len < 0) len = (int)strlen(str) + 1;
	wchar_t *widestr = new wchar_t[len];
	if (MultiByteToWideChar(CP_ACP, 0, str, len, widestr, len) == 0) {
		delete[] widestr;
		throw ref new Platform::InvalidArgumentException();
	}
	Platform::String^ ret = ref new Platform::String(widestr);
	delete[] widestr;
	return ret;
}

namespace Vorbisfile {

	namespace WindowsRuntime {

		public ref class VorbisComment sealed {
		public:
			property Platform::String^ Vendor {
				Platform::String^ get() { return vendor_ ? vendor_ : (vendor_ = string_from_utf8(vc_->vendor)); }
			}

			property Platform::Array<Platform::String^>^ Comments {
				Platform::Array<Platform::String^>^ get() {
					return user_comments_ ? user_comments_ : (user_comments_ = InitializeUserComments());
				}
			}

		internal:
			VorbisComment(const ::vorbis_comment *vc) : vc_(vc) { }

		private:
			inline Platform::Array<Platform::String^>^ InitializeUserComments() {
				Platform::Array<Platform::String^>^ arr = ref new Platform::Array<Platform::String^>((unsigned)vc_->comments);
				for (unsigned i = 0; i < arr->Length; i++) {
					arr[i] = string_from_utf8(vc_->user_comments[i], vc_->comment_lengths[i]);
				}
				return arr;
			}

			const ::vorbis_comment *vc_;

			Platform::String^ vendor_;
			Platform::Array<Platform::String^>^ user_comments_;
		};


		public ref class VorbisInfo sealed {
		public:
			property int Version {
				int get() { return vi_->version; }
			}

			property int Channels {
				int get() { return vi_->channels; }
			}

			property int Rate {
				int get() { return vi_->rate; }
			}

			property int BitrateUpper {
				int get() { return vi_->bitrate_upper; }
			}

			property int BitrateNominal {
				int get() { return vi_->bitrate_nominal; }
			}

			property int BitrateLower {
				int get() { return vi_->bitrate_lower; }
			}

			property int BitrateWindow {
				int get() { return vi_->bitrate_window; }
			}

		internal:
			VorbisInfo(const ::vorbis_info *vi) : vi_(vi) { }

		private:
			const ::vorbis_info *vi_;
		};


		public ref class OggVorbisFile sealed {
		public:
			OggVorbisFile();
			virtual ~OggVorbisFile();

			/* Call after open to check that the object was created
			 * successfully.  If not, use Open() to try again.
			 */
			property bool IsValid { bool get(); }

			void Open(Windows::Storage::Streams::IRandomAccessStream^ fileStream);
			void Open(Windows::Storage::Streams::IRandomAccessStream^ fileStream, Windows::Storage::Streams::IBuffer^ initial);
			void Clear();

			static OggVorbisFile^ TestOpen(Windows::Storage::Streams::IRandomAccessStream^ fileStream);
			static OggVorbisFile^ TestOpen(Windows::Storage::Streams::IRandomAccessStream^ fileStream, Windows::Storage::Streams::IBuffer^ initial);

			Windows::Storage::Streams::IBuffer^ Read(int length);
			Windows::Storage::Streams::IBuffer^ Read(int length, int *bitstream);

			static void Crosslap(OggVorbisFile^ oldFile, OggVorbisFile^ newFile);

			void RawSeek(ogg_int64_t pos);
			void PcmSeek(ogg_int64_t pos);
			void TimeSeek(double pos);
			void PcmSeekPage(ogg_int64_t pos);
			void TimeSeekPage(double pos);

			void RawSeekLap(ogg_int64_t pos);
			void PcmSeekLap(ogg_int64_t pos);
			void TimeSeekLap(double pos);
			void PcmSeekPageLap(ogg_int64_t pos);
			void TimeSeekPageLap(double pos);

			int Bitrate();
			int Bitrate(int bitstream);
			int BitrateInstant();
			int Streams();
			bool Seekable();
			int SerialNumber();
			int SerialNumber(int bitstream);
			ogg_int64_t RawTotal();
			ogg_int64_t RawTotal(int bitstream);
			ogg_int64_t PcmTotal();
			ogg_int64_t PcmTotal(int bitstream);
			double TimeTotal();
			double TimeTotal(int bitstream);
			ogg_int64_t RawTell();
			ogg_int64_t PcmTell();
			double TimeTell();
			VorbisInfo^ Info();
			VorbisInfo^ Info(int bitstream);
			VorbisComment^ Comment();
			VorbisComment^ Comment(int bitstream);

		private:
			::OggVorbis_File *vf_;
			Windows::Storage::Streams::IRandomAccessStream^ file_stream_;
			Windows::Storage::Streams::DataReader^ file_reader_;

			static size_t read_func_(void *ptr, size_t size, size_t nmemb, void *datasource);
			static int seek_func_(void *datasource, ogg_int64_t offset, int whence);
			static int close_func_(void *datasource);
			static long tell_func_(void *datasource);
		};

	}

}
