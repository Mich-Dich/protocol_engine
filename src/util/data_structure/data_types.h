
#pragma once

#include <inttypes.h>

// TYPES ===========================================================================================================

typedef uint8_t 					u8;		// 8-bit unsigned integer
typedef uint16_t 					u16;	// 16-bit unsigned integer
typedef uint32_t 					u32;	// 32-bit unsigned integer
typedef uint64_t 					u64;	// 64-bit unsigned integer

typedef int8_t 						i8;	 	// 8-bit signed integer
typedef int16_t 					i16; 	// 16-bit signed integer
typedef int32_t 					i32; 	// 32-bit signed integer
typedef int64_t 					i64; 	// 64-bit signed integer

typedef float 						f32;	// 32-bit floating point
typedef double 						f64;	// 64-bit floating point
typedef long double 				f128;	// 128-bit floating point (platform dependent)

// Platform-specific types
typedef u64 						handle; // Generic handle type for OS resources

constexpr handle invalid_handle = 0U;

// FORWARD DECLARATIONS ================================================================================================


namespace PE {

	// CONSTANTS =======================================================================================================

	// MACROS ==========================================================================================================

	// TYPES ===========================================================================================================

	template<typename E>
	constexpr auto to_base(E e) noexcept 		{ return static_cast<std::underlying_type_t<E>>(e); }

	// @brief Type trait to detect std::vector types
	template <typename T>
	struct is_vector : std::false_type {};


	template <typename T, typename Alloc>
	struct is_vector<std::vector<T, Alloc>> : std::true_type {};


	// @brief Fixed-size character array type
	template <size_t N>
	struct char_array { char data[N]; };

	// smart pointer ---------------------------------------------------------------------------------------------------

	// @brief [ref] is a reference-counted smart pointer similar to std::shared_ptr
	template <typename T>
	using ref = std::shared_ptr<T>;


	// @brief [scope_ref] is an owning smart pointer similar to std::unique_ptr
	template <typename T>
	using unique_ref = std::unique_ptr<T>;


	// @brief [weak_ref] is a reference-counted smart pointer similar to std::weak_ptr
	template <typename T>
	class weak_ref {
	public:

		weak_ref() = default;															// Default constructor

		weak_ref(const ref<T>& shared) : m_weak_ptr(shared) {}							// Constructor from shared_ptr

		// CAUTION, this moves ownership!
		template <typename U = T>
		weak_ref(unique_ref<U>&& unique) : m_weak_ptr(ref<U>(std::move(unique))) {}		// Constructor from unique_ref - converts to shared_ptr first

		// Constructor that creates a weak_ref from unique_ref without transferring ownership
		// This creates an "aliasing" weak_ptr that doesn't own the object
		weak_ref(const unique_ref<T>& unique) : m_weak_ptr(std::shared_ptr<T>(std::shared_ptr<T>{}, unique.get())) {}

		weak_ref(const std::weak_ptr<T>& weak) : m_weak_ptr(weak) {}					// Constructor from weak_ptr

		weak_ref(T* ptr) = delete;  // Forbid this usage
		// // Or make it explicit that it's only for stack objects
		// template<typename T, typename = std::enable_if_t<std::is_array_v<T> || !std::is_pointer_v<std::remove_reference_t<T>>>>
		// weak_ref(T* ptr) {
		// 	static_assert(false, "weak_ref cannot be created from raw pointers to heap objects");
		// }

		operator std::weak_ptr<T>() const									{ return m_weak_ptr; }								// Conversion to std::weak_ptr

		bool expired() const												{ return m_weak_ptr.expired(); }					// Delegate other weak_ptr operations

		ref<T> lock() const													{ return m_weak_ptr.lock(); }

		bool operator==(const std::weak_ptr<T>& other) const 				{ return m_weak_ptr.lock() == other.lock(); }		// Allow comparison with std::weak_ptr

	private:

		std::weak_ptr<T> m_weak_ptr;

	};


	// @brief Creates a reference-counted object with perfect forwarding
	template <typename T, typename... args>
	constexpr ref<T> create_ref(args &&...arguments)						{ return std::make_shared<T>(std::forward<args>(arguments)...); }


	// Create weak reference from existing shared reference
	template <typename T>
	constexpr weak_ref<T> create_weak_ref(const ref<T>& shared)				{ return shared; } 	// Implicit conversion from shared_ptr to weak_ptr


	// @brief Creates a scoped object with perfect forwarding
	template <typename T, typename... args>
	constexpr unique_ref<T> create_unique_ref(args &&...arguments)			{ return std::make_unique<T>(std::forward<args>(arguments)...); }

	// smart pointer ---------------------------------------------------------------------------------------------------
	//  ---------------------------------------------------------------------------------------------------

	// @brief Semantic versioning structure
	struct version {

		version() {}
		version(u16 major, u16 minor, u16 patch)
			: major(major), minor(minor), patch(patch) {}

		u16 			major{}; 		// Major version number (incompatible API changes)
		u16 			minor{}; 		// Minor version number (backwards-compatible functionality)
		u16 			patch{}; 		// Patch version number (backwards-compatible bug fixes)

		// @brief Converts version to string in "major:minor:patch" format
		std::string to_str() const 											{ return std::format("v{}.{}.{}", major, minor, patch); }

		// @brief Implicit conversion to string_view
		operator std::string_view() 										{ return std::format("v{}.{}.{}", major, minor, patch); }
	};


	// @brief Stream output operator for version
	inline std::ostream& operator<<(std::ostream& os, const version& v) 	{ return os << v.to_str(); }


	// @brief System time representation
	struct system_time {

		u16 			year;			// Full year (e.g., 2025)
		u8 				month;		 	// Month (1-12)
		u8 				day;			// Day of month (1-31)
		u8 				day_of_week;	// Day of week (0-6, where 0=Sunday)
		u8 				hour;		 	// Hour (0-23)
		u8 				minute;		 	// Minute (0-59)
		u8 				secund;		 	// Second (0-59)
		u16 			millisecond; 	// Millisecond (0-999)

		// “older than”
		bool operator<(const system_time& other) const {
			return std::tie(year, month, day, hour, minute, secund, millisecond) < std::tie(other.year, other.month, other.day,
				other.hour, other.minute, other.secund, other.millisecond);
		}


		// “newer than”
		bool operator>(const system_time& other) const						{ return other < *this; }


		// “not newer than” (i.e. older or equal)
		bool operator<=(const system_time& other) const						{ return !(*this > other); }


		// “not older than” (i.e. newer or equal)
		bool operator>=(const system_time& other) const						{ return !(*this < other); }


		// equality
		bool operator==(const system_time& other) const {
			return std::tie(year, month, day, day_of_week, hour, minute, secund, millisecond) == std::tie(other.year, other.month, other.day,
				other.day_of_week, other.hour, other.minute, other.secund, other.millisecond);
		}


		// inequality
		bool operator!=(const system_time& other) const						{ return !(*this == other); }


		// @brief Converts system_time to human-readable string
		std::string to_str() const { return std::format("{}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}", year, month, day, hour, minute, secund, millisecond); }

		// @brief Check if this time is older than another time by at least the specified duration
		// @param other The time to compare against
		// @param seconds The minimum number of seconds that must have passed
		// @return true if this time is older than the other time by at least the specified seconds
		bool is_older_than(const system_time& other, u32 seconds) const {
			// First check if this time is actually older using existing operator
			if (*this >= other) return false;

			// Calculate difference in seconds
			i64 diff_seconds = 0;
			diff_seconds += (other.year - year) * 365 * 24 * 60 * 60;
			diff_seconds += (other.month - month) * 30 * 24 * 60 * 60;
			diff_seconds += (other.day - day) * 24 * 60 * 60;
			diff_seconds += (other.hour - hour) * 60 * 60;
			diff_seconds += (other.minute - minute) * 60;
			diff_seconds += (other.secund - secund);
			f64 total_diff = static_cast<f64>(diff_seconds) + (static_cast<f64>(other.millisecond) - static_cast<f64>(millisecond)) / 1000.0;

			return total_diff >= seconds;
		}

		// @brief Check if this time is older than another time by at least the specified duration
		// This version allows specifying minutes instead of seconds
		bool is_older_than_minutes(const system_time& other, u32 minutes) const {
			return is_older_than(other, minutes * 60);
		}
	};

	// enums -----------------------------------------------------------------------------------------------------------

	// @brief Precision levels for duration measurements
	enum class time_unit : u8 {

		microseconds = 0,
		milliseconds,
		seconds,
	};


	// @brief System state enumeration
	enum class system_state : u8 {
		idle = 0,
		running,
		paused,
		destroyed,
	};

	// STATIC VARIABLES ================================================================================================

	// FUNCTION DECLARATION ============================================================================================

	// TEMPLATE DECLARATION ============================================================================================

	// CLASS DECLARATION ===============================================================================================

}
