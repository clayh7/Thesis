#pragma once

#include <vector>
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/UISystem/WidgetProperty.hpp"
#include "Engine/Math/Vector2f.hpp"


//-------------------------------------------------------------------------------------------------
class UIWidgetRegistration;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
class UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static UIWidgetRegistration s_UIWidgetRegistration;
	static size_t const INVALID_UID;
	static char const * PROPERTY_ANCHOR;
	static char const * PROPERTY_DOCK;
	static char const * PROPERTY_OFFSET;
	static char const * PROPERTY_WIDTH;
	static char const * PROPERTY_HEIGHT;
	static char const * PROPERTY_SOUND;
	static char const * PROPERTY_ON_HIGHLIGHT;
	static char const * PROPERTY_ON_PRESSED;
	static char const * PROPERTY_ON_RELEASED;

protected:
	static char const * PROPERTY_HIDDEN;
	static char const * STRING_VALUE;
	static char const * STRING_STATE;

private:
	static size_t s_currentUID;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static UIWidget * CreateWidgetFromXML( XMLNode const & node );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
protected:
	UIWidget * m_parent;
	std::vector<UIWidget*> m_children;
	std::string m_name;
	eWidgetState m_state;
	bool m_hidden;
	bool m_dirty;
	size_t const m_uid; //unique user interface id
	//I need to use this is get size, to store the last know value so we don't recalculate it every frame
	mutable Vector2f m_lastSize;
	mutable bool m_recalculateSize;
	NamedProperties m_properties[eWidgetState_COUNT];

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UIWidget( std::string const & name = "" );
	UIWidget( XMLNode const & node );
	virtual ~UIWidget( );

	virtual void Update( );
	virtual void Render( ) const;
	void AddChild( UIWidget * widget );
	void RemoveChild( UIWidget * widget );
	UIWidget * GetChild( size_t childIndex );
	void ChangeState( eWidgetState const & state );
	void Highlight( );
	void Unhighlight( );
	void Press( );
	void Release( );
	void Disable( );
	void Enable( );
	void Dirty( );

	UIWidget * FindWidgetUnderPosition( Vector2f const & position );
	UIWidget * GetParent( ) const;
	virtual Vector2f GetSize( ) const; //overriden in UIItem
	eWidgetState GetState( ) const;
	size_t GetUID( ) const;
	
	void SetParent( UIWidget * widget );
	void SetHidden( bool isHidden );

protected:
	void EnterState( eWidgetState const & state );
	void ExitState( eWidgetState const & state );
	void UpdateState( eWidgetState const & state );
	void PopulateFromXML( XMLNode const & node );
	void PlayUISound( );
	
	bool IsPointInside( Vector2f const & position ) const;
	Vector2f GetSizeForChild( UIWidget const * forChild = nullptr ) const;
	Vector2f GetLocalPosition( eAnchor const & anchor ) const;
	virtual Vector2f GetWorldPosition( eAnchor const & anchor, UIWidget const * forChild = nullptr ) const; //overriden in UIItem
	eAnchor GetAnchor( ) const;
	Vector2f GetOffset( ) const;
	eDock GetDock( ) const;
	bool IsSelected( ) const;

//-------------------------------------------------------------------------------------------------
// Function Templates
//-------------------------------------------------------------------------------------------------
public:
	//Specific Override (strings)
	void SetProperty( std::string const & propertyName, char const * propertyValue )
	{
		SetProperty( propertyName, std::string( propertyValue ), eWidgetPropertySource_WIDGET_SPECIFIC, eWidgetState_ALL );
	}

	//-------------------------------------------------------------------------------------------------
	template<typename T>
	void SetProperty( std::string const & propertyName, T const & propertyValue )
	{
		SetProperty( propertyName, propertyValue, eWidgetPropertySource_WIDGET_SPECIFIC, eWidgetState_ALL );
	}
	
	//-------------------------------------------------------------------------------------------------
	template<typename T>
	void SetProperty( std::string const & propertyName, T const & propertyValue, eWidgetState state )
	{
		SetProperty( propertyName, propertyValue, eWidgetPropertySource_WIDGET_SPECIFIC, state );
	}

	//-------------------------------------------------------------------------------------------------
	//Specific Override (strings)
	void SetProperty( std::string const & propertyName, char const * propertyValue, eWidgetPropertySource source )
	{
		SetProperty( propertyName, std::string( propertyValue ), source, eWidgetState_ALL );
	}

	//-------------------------------------------------------------------------------------------------
	template<typename T>
	void SetProperty( std::string const & propertyName, T const & propertyValue, eWidgetPropertySource source )
	{
		SetProperty( propertyName, propertyValue, source, eWidgetState_ALL );
	}

	//-------------------------------------------------------------------------------------------------
	template<typename T>
	void SetProperty( std::string const & propertyName, T const & propertyValue,
		eWidgetPropertySource source, eWidgetState state )
	{
		WidgetProperty<T> propertyData( propertyValue, source, state );
		if( IsHigherPriority( propertyName, propertyData ) )
		{
			m_properties[state].Set( propertyName, propertyData );
			Dirty( );
		}
	}

	//-------------------------------------------------------------------------------------------------
	//Higher priority means we want to keep it, lower means we don't care so don't save it
	template<typename T>
	bool IsHigherPriority( std::string const & propertyName, WidgetProperty<T> const & newProperty )
	{
		WidgetProperty<T> currentValue;
		ePropertyGetResult result = m_properties[newProperty.m_state].Get( propertyName, currentValue );
		ASSERT_RECOVERABLE( result != ePropertyGetResult_FAILED_WRONG_TYPE, "Widget Property doesn't match current type" );

		//Doesn't exist yet, keep it
		if( result == ePropertyGetResult_FAILED_NOT_PRESENT )
		{
			return true;
		}
		if( result == ePropertyGetResult_SUCCESS )
		{
			if( newProperty.m_source < currentValue.m_source )
			{
				return false;
			}
		}
		return true;
	}

	//-------------------------------------------------------------------------------------------------
	template<typename T>
	void GetProperty( std::string const & propertyName, T & out_propertyValue ) const
	{
		//Try to get more specific property
		WidgetProperty<T> widgetProperty;
		ePropertyGetResult result = m_properties[m_state].Get( propertyName, widgetProperty );

		if( result != ePropertyGetResult_FAILED_NOT_PRESENT )
		{
			out_propertyValue = widgetProperty.m_value;
		}
		//Default to generic property
		else
		{
			result = m_properties[eWidgetState_ALL].Get( propertyName, widgetProperty );
			if( result != ePropertyGetResult_FAILED_NOT_PRESENT )
			{
				out_propertyValue = widgetProperty.m_value;
			}
		}
	}


	//-------------------------------------------------------------------------------------------------
	template<typename T>
	T * FindWidgetByName( std::string const & name )
	{
		if( m_name == name )
		{
			T * widget = static_cast<T*>( this );
			return widget;
		}

		//Go through children, find the first one that matches the name
		for( UIWidget * child : m_children )
		{
			T * foundWidget = child->FindWidgetByName<T>( name );
			if( foundWidget )
			{
				return foundWidget;
			}
		}

		return nullptr;
	}
};