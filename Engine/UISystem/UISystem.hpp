#pragma once
#include <string>
#include <map>


//-------------------------------------------------------------------------------------------------
class MeshBuilder;
class NamedProperties;
class SpriteGameRenderer;
class UIItem;
class UISystem;
class UIWidget;
class Vector2f;
class Vector3f;
struct XMLNode;
typedef UIWidget * (WidgetCreationFunc) ( XMLNode const & widgetData );


//-------------------------------------------------------------------------------------------------
class UIWidgetRegistration
{
//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UIWidgetRegistration( std::string const & widgetName, WidgetCreationFunc * creationFunc );
};


//-------------------------------------------------------------------------------------------------
extern UISystem * g_UISystem;


//-------------------------------------------------------------------------------------------------
class UISystem
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * DEFAULT_NAME;
	static char const * UI_SKIN;
	static std::map<size_t, WidgetCreationFunc*> * s_registeredWidgets;
	static int const VIRTUAL_WIDTH = 1600;
	static int const VIRTUAL_HEIGHT = 900;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static Vector2f ClipToUISystemPosition( Vector3f const & screenVector );
	static void RegisterWidget( std::string const & widgetName, WidgetCreationFunc * creationFunc );
	static UIWidget * CreateWidgetFromName( std::string const & name, XMLNode const & data );
	static Vector2f GetCursorUIPosition( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	UIWidget * m_root;
	UIWidget * m_highlightedWidget;
	UIWidget * m_selectedWidget;
	std::map<size_t, XMLNode> m_skins;
	SpriteGameRenderer * m_uiSpriteRenderer;
	UIItem * m_heldItem;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UISystem( );
	~UISystem( );
	void UpdateUISpriteRenderer( );
	void LoadUIFromXML( );
	void Update( );
	void Render( ) const;
	UIWidget * CreateWidget( );
	void RemoveWidget( UIWidget * widget );

	UIWidget * GetHighlightedWidget( ) const;
	UIWidget * GetSelectedWidget( ) const;
	UIItem * GetHeldItem( ) const;
	SpriteGameRenderer const * GetRenderer( ) const;
	UIWidget * GetWidgetUnderCursor( ) const;

	void SetHeldItem( UIItem * item );
	void Select( UIWidget * selectedWidget );

private:
	bool IsHoldingAnItem( ) const;
	void AddSkinFromXML( XMLNode const & node );
	void AddWidgetFromXML( XMLNode const & node );

//-------------------------------------------------------------------------------------------------
// Function Templates
//-------------------------------------------------------------------------------------------------
public:
	template<typename T>
	T * CreateWidget( std::string const & name = DEFAULT_NAME )
	{
		T * widget = new T( name );
		m_root->AddChild( widget );
		return widget;
	}

	//-------------------------------------------------------------------------------------------------
	template<typename T>
	T * GetWidgetByName( std::string const & name )
	{
		if( strcmp( name.c_str( ), DEFAULT_NAME ) == 0 )
		{
			return nullptr;
		}

		return m_root->FindWidgetByName<T>( name );
	}
};